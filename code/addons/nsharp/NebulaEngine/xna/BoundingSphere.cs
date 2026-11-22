
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
namespace Mathf
{
    [DataContract]
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    public struct BoundingSphere : IEquatable<BoundingSphere>
    {
        #region Public Fields
        [DataMember]
        public Vector3 Center;
        [DataMember]
        public float Radius;
        #endregion
        #region Internal Properties
        internal string DebugDisplayString
        {
            get
            {
                return string.Concat(
                    "Центр( ", this.Center.DebugDisplayString, " ) \ г \ п",
                    "Радиус( ", this.Radius.ToString(), " )"
                    );
            }
        }
        #endregion
        #region Constructors
        public BoundingSphere(Vector3 center, float radius)
        {
            this.Center = center;
            this.Radius = radius;
        }
        #endregion
        #region Public Methods
        #region Contains
        public ContainmentType Contains(BoundingBox box)
        {
            bool inside = true;
            foreach (Vector3 corner in box.GetCorners())
            {
                if (this.Contains(corner) == ContainmentType.Disjoint)
                {
                    inside = false;
                    break;
                }
            }
            if (inside)
                return ContainmentType.Contains;
            double dmin = 0;
            if (Center.X < box.Min.X)
				dmin += (Center.X - box.Min.X) * (Center.X - box.Min.X);
			else if (Center.X > box.Max.X)
					dmin += (Center.X - box.Max.X) * (Center.X - box.Max.X);
			if (Center.Y < box.Min.Y)
				dmin += (Center.Y - box.Min.Y) * (Center.Y - box.Min.Y);
			else if (Center.Y > box.Max.Y)
				dmin += (Center.Y - box.Max.Y) * (Center.Y - box.Max.Y);
			if (Center.Z < box.Min.Z)
				dmin += (Center.Z - box.Min.Z) * (Center.Z - box.Min.Z);
			else if (Center.Z > box.Max.Z)
				dmin += (Center.Z - box.Max.Z) * (Center.Z - box.Max.Z);
			if (dmin <= Radius * Radius)
				return ContainmentType.Intersects;
            return ContainmentType.Disjoint;
        }
        public void Contains(ref BoundingBox box, out ContainmentType result)
        {
            result = this.Contains(box);
        }
        public ContainmentType Contains(BoundingFrustum frustum)
        {
            bool inside = true;
            Vector3[] corners = frustum.GetCorners();
            foreach (Vector3 corner in corners)
            {
                if (this.Contains(corner) == ContainmentType.Disjoint)
                {
                    inside = false;
                    break;
                }
            }
            if (inside)
                return ContainmentType.Contains;
            double dmin = 0;
            if (dmin <= Radius * Radius)
                return ContainmentType.Intersects;
            return ContainmentType.Disjoint;
        }
        public void Contains(ref BoundingFrustum frustum,out ContainmentType result)
        {
            result = this.Contains(frustum);
        }
        public ContainmentType Contains(BoundingSphere sphere)
        {
            ContainmentType result;
            Contains(ref sphere, out result);
            return result;
        }
        public void Contains(ref BoundingSphere sphere, out ContainmentType result)
        {
            float sqDistance;
            Vector3.DistanceSquared(ref sphere.Center, ref Center, out sqDistance);
            if (sqDistance > (sphere.Radius + Radius) * (sphere.Radius + Radius))
                result = ContainmentType.Disjoint;
            else if (sqDistance <= (Radius - sphere.Radius) * (Radius - sphere.Radius))
                result = ContainmentType.Contains;
            else
                result = ContainmentType.Intersects;
        }
        public ContainmentType Contains(Vector3 point)
        {
            ContainmentType result;
            Contains(ref point, out result);
            return result;
        }
        public void Contains(ref Vector3 point, out ContainmentType result)
        {
            float sqRadius = Radius * Radius;
            float sqDistance;
            Vector3.DistanceSquared(ref point, ref Center, out sqDistance);
            if (sqDistance > sqRadius)
                result = ContainmentType.Disjoint;
            else if (sqDistance < sqRadius)
                result = ContainmentType.Contains;
            else
                result = ContainmentType.Intersects;
        }
        #endregion
        #region CreateFromBoundingBox
        public static BoundingSphere CreateFromBoundingBox(BoundingBox box)
        {
            BoundingSphere result;
            CreateFromBoundingBox(ref box, out result);
            return result;
        }
        public static void CreateFromBoundingBox(ref BoundingBox box, out BoundingSphere result)
        {
            Vector3 center = new Vector3((box.Min.X + box.Max.X) / 2.0f,
                                         (box.Min.Y + box.Max.Y) / 2.0f,
                                         (box.Min.Z + box.Max.Z) / 2.0f);
            float radius = Vector3.Distance(center, box.Max);
            result = new BoundingSphere(center, radius);
        }
        #endregion
        public static BoundingSphere CreateFromFrustum(BoundingFrustum frustum)
        {
            return CreateFromPoints(frustum.GetCorners());
        }
        public static BoundingSphere CreateFromPoints(IEnumerable<Vector3> points)
        {
            if (points == null )
                throw new ArgumentNullException("points");
            var minx = new Vector3(float.MaxValue, float.MaxValue, float.MaxValue);
            var maxx = -minx;
            var miny = minx;
            var maxy = -minx;
            var minz = minx;
            var maxz = -minx;
            var numPoints = 0;
            foreach (var pt in points)
            {
                ++numPoints;
                if (pt.X < minx.X)
                    minx = pt;
                if (pt.X > maxx.X)
                    maxx = pt;
                if (pt.Y < miny.Y)
                    miny = pt;
                if (pt.Y > maxy.Y)
                    maxy = pt;
                if (pt.Z < minz.Z)
                    minz = pt;
                if (pt.Z > maxz.Z)
                    maxz = pt;
            }
            if (numPoints == 0)
                throw new ArgumentException("У вас должно быть хотя бы одно очко в очках.");
            var sqDistX = Vector3.DistanceSquared(maxx, minx);
            var sqDistY = Vector3.DistanceSquared(maxy, miny);
            var sqDistZ = Vector3.DistanceSquared(maxz, minz);
            var min = minx;
            var max = maxx;
            if (sqDistY > sqDistX && sqDistY > sqDistZ)
            {
                max = maxy;
                min = miny;
            }
            if (sqDistZ > sqDistX && sqDistZ > sqDistY)
            {
                max = maxz;
                min = minz;
            }
            var center = (min + max) * 0.5f;
            var radius = Vector3.Distance(max, center);
            float sqRadius = radius * radius;
            foreach (var pt in points)
            {
                Vector3 diff = (pt-center);
                float sqDist = diff.LengthSquared();
                if (sqDist > sqRadius)
                {
                    float distance = (float)Math.Sqrt(sqDist);
                    Vector3 direction = diff / distance;
                    Vector3 G = center - radius * direction;
                    center = (G + pt) / 2;
                    radius = Vector3.Distance(pt, center);
                    sqRadius = radius * radius;
                }
            }
            return new BoundingSphere(center, radius);
        }
        public static BoundingSphere CreateMerged(BoundingSphere original, BoundingSphere additional)
        {
            BoundingSphere result;
            CreateMerged(ref original, ref additional, out result);
            return result;
        }
        public static void CreateMerged(ref BoundingSphere original, ref BoundingSphere additional, out BoundingSphere result)
        {
            Vector3 ocenterToaCenter = Vector3.Subtract(additional.Center, original.Center);
            float distance = ocenterToaCenter.Length();
            if (distance <= original.Radius + additional.Radius)
            {
                if (distance <= original.Radius - additional.Radius)
                {
                    result = original;
                    return;
                }
                if (distance <= additional.Radius - original.Radius)
                {
                    result = additional;
                    return;
                }
            }
            float leftRadius = Math.Max(original.Radius - distance, additional.Radius);
            float Rightradius = Math.Max(original.Radius + distance, additional.Radius);
            ocenterToaCenter = ocenterToaCenter + (((leftRadius - Rightradius) / (2 * ocenterToaCenter.Length())) * ocenterToaCenter);
            result = new BoundingSphere();
            result.Center = original.Center + ocenterToaCenter;
            result.Radius = (leftRadius + Rightradius) / 2;
        }
        public bool Equals(BoundingSphere other)
        {
            return this.Center == other.Center && this.Radius == other.Radius;
        }
        public override bool Equals(object obj)
        {
            if (obj is BoundingSphere)
                return this.Equals((BoundingSphere)obj);
            return false;
        }
        public override int GetHashCode()
        {
            return this.Center.GetHashCode() + this.Radius.GetHashCode();
        }
        #region Intersects
        public bool Intersects(BoundingBox box)
        {
			return box.Intersects(this);
        }
        public void Intersects(ref BoundingBox box, out bool result)
        {
            box.Intersects(ref this, out result);
        }
        public bool Intersects(BoundingSphere sphere)
        {
            bool result;
            Intersects(ref sphere, out result);
            return result;
        }
        public void Intersects(ref BoundingSphere sphere, out bool result)
        {
            float sqDistance;
            Vector3.DistanceSquared(ref sphere.Center, ref Center, out sqDistance);
            if (sqDistance > (sphere.Radius + Radius) * (sphere.Radius + Radius))
                result = false;
            else
                result = true;
        }
        public PlaneIntersectionType Intersects(Plane plane)
        {
            var result = default(PlaneIntersectionType);
            this.Intersects(ref plane, out result);
            return result;
        }
        public void Intersects(ref Plane plane, out PlaneIntersectionType result)
        {
            var distance = default(float);
            Vector3.Dot(ref plane.Normal, ref this.Center, out distance);
            distance += plane.D;
            if (distance > this.Radius)
                result = PlaneIntersectionType.Front;
            else if (distance < -this.Radius)
                result = PlaneIntersectionType.Back;
            else
                result = PlaneIntersectionType.Intersecting;
        }
        public float? Intersects(Ray ray)
        {
            return ray.Intersects(this);
        }
        public void Intersects(ref Ray ray, out float? result)
        {
            ray.Intersects(ref this, out result);
        }
        #endregion
        public override string ToString()
        {
            return "{Центр:" + this.Center + " Radius:" + this.Radius + "}";
        }
        #region Transform
        public BoundingSphere Transform(Matrix matrix)
        {
            BoundingSphere sphere = new BoundingSphere();
            sphere.Center = Vector3.Transform(this.Center, matrix);
            sphere.Radius = this.Radius * ((float)Math.Sqrt((double)Math.Max(((matrix.M11 * matrix.M11) + (matrix.M12 * matrix.M12)) + (matrix.M13 * matrix.M13), Math.Max(((matrix.M21 * matrix.M21) + (matrix.M22 * matrix.M22)) + (matrix.M23 * matrix.M23), ((matrix.M31 * matrix.M31) + (matrix.M32 * matrix.M32)) + (matrix.M33 * matrix.M33)))));
            return sphere;
        }
        public void Transform(ref Matrix matrix, out BoundingSphere result)
        {
            result.Center = Vector3.Transform(this.Center, matrix);
            result.Radius = this.Radius * ((float)Math.Sqrt((double)Math.Max(((matrix.M11 * matrix.M11) + (matrix.M12 * matrix.M12)) + (matrix.M13 * matrix.M13), Math.Max(((matrix.M21 * matrix.M21) + (matrix.M22 * matrix.M22)) + (matrix.M23 * matrix.M23), ((matrix.M31 * matrix.M31) + (matrix.M32 * matrix.M32)) + (matrix.M33 * matrix.M33)))));
        }
        #endregion
        public void Deconstruct(out Vector3 center, out float radius)
        {
            center = Center;
            radius = Radius;
        }
        #endregion
        #region Operators
        public static bool operator == (BoundingSphere a, BoundingSphere b)
        {
            return a.Equals(b);
        }
        public static bool operator != (BoundingSphere a, BoundingSphere b)
        {
            return !a.Equals(b);
        }
        #endregion
    }
}