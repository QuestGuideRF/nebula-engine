
using System;
using System.Diagnostics;
using System.Runtime.Serialization;
namespace Mathf
{
    [DataContract]
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    public struct Ray : IEquatable<Ray>
    {
        #region Public Fields
        [DataMember]
        public Vector3 Direction;
        [DataMember]
        public Vector3 Position;
        #endregion
        #region Public Constructors
        public Ray(Vector3 position, Vector3 direction)
        {
            this.Position = position;
            this.Direction = direction;
        }
        #endregion
        #region Public Methods
        public override bool Equals(object obj)
        {
            return (obj is Ray) ? this.Equals((Ray)obj) : false;
        }
        public bool Equals(Ray other)
        {
            return this.Position.Equals(other.Position) && this.Direction.Equals(other.Direction);
        }
        public override int GetHashCode()
        {
            return Position.GetHashCode() ^ Direction.GetHashCode();
        }
        public float? Intersects(BoundingBox box)
        {
            const float Epsilon = 1e-6f;
            float? tMin = null, tMax = null;
            if (Math.Abs(Direction.X) < Epsilon)
            {
                if (Position.X < box.Min.X || Position.X > box.Max.X)
                    return null;
            }
            else
            {
                tMin = (box.Min.X - Position.X) / Direction.X;
                tMax = (box.Max.X - Position.X) / Direction.X;
                if (tMin > tMax)
                {
                    var temp = tMin;
                    tMin = tMax;
                    tMax = temp;
                }
            }
            if (Math.Abs(Direction.Y) < Epsilon)
            {
                if (Position.Y < box.Min.Y || Position.Y > box.Max.Y)
                    return null;
            }
            else
            {
                var tMinY = (box.Min.Y - Position.Y) / Direction.Y;
                var tMaxY = (box.Max.Y - Position.Y) / Direction.Y;
                if (tMinY > tMaxY)
                {
                    var temp = tMinY;
                    tMinY = tMaxY;
                    tMaxY = temp;
                }
                if ((tMin.HasValue && tMin > tMaxY) || (tMax.HasValue && tMinY > tMax))
                    return null;
                if (!tMin.HasValue || tMinY > tMin) tMin = tMinY;
                if (!tMax.HasValue || tMaxY < tMax) tMax = tMaxY;
            }
            if (Math.Abs(Direction.Z) < Epsilon)
            {
                if (Position.Z < box.Min.Z || Position.Z > box.Max.Z)
                    return null;
            }
            else
            {
                var tMinZ = (box.Min.Z - Position.Z) / Direction.Z;
                var tMaxZ = (box.Max.Z - Position.Z) / Direction.Z;
                if (tMinZ > tMaxZ)
                {
                    var temp = tMinZ;
                    tMinZ = tMaxZ;
                    tMaxZ = temp;
                }
                if ((tMin.HasValue && tMin > tMaxZ) || (tMax.HasValue && tMinZ > tMax))
                    return null;
                if (!tMin.HasValue || tMinZ > tMin) tMin = tMinZ;
                if (!tMax.HasValue || tMaxZ < tMax) tMax = tMaxZ;
            }
            if ((tMin.HasValue && tMin < 0) && tMax > 0) return 0;
            if (tMin < 0) return null;
            return tMin;
        }
        public void Intersects(ref BoundingBox box, out float? result)
        {
			result = Intersects(box);
        }
        public float? Intersects(BoundingSphere sphere)
        {
            float? result;
            Intersects(ref sphere, out result);
            return result;
        }
        public float? Intersects(Plane plane)
        {
            float? result;
            Intersects(ref plane, out result);
            return result;
        }
        public void Intersects(ref Plane plane, out float? result)
        {
            var den = Vector3.Dot(Direction, plane.Normal);
            if (Math.Abs(den) < 0.00001f)
            {
                result = null;
                return;
            }
            result = (-plane.D - Vector3.Dot(plane.Normal, Position)) / den;
            if (result < 0.0f)
            {
                if (result < -0.00001f)
                {
                    result = null;
                    return;
                }
                result = 0.0f;
            }
        }
        public void Intersects(ref BoundingSphere sphere, out float? result)
        {
            Vector3 difference = sphere.Center - this.Position;
            float differenceLengthSquared = difference.LengthSquared();
            float sphereRadiusSquared = sphere.Radius * sphere.Radius;
            float distanceAlongRay;
            if (differenceLengthSquared < sphereRadiusSquared)
            {
                result = 0.0f;
                return;
            }
            Vector3.Dot(ref this.Direction, ref difference, out distanceAlongRay);
            if (distanceAlongRay < 0)
            {
                result = null;
                return;
            }
            float dist = sphereRadiusSquared + distanceAlongRay * distanceAlongRay - differenceLengthSquared;
            result = (dist < 0) ? null : distanceAlongRay - (float?)Math.Sqrt(dist);
        }
        public static bool operator !=(Ray a, Ray b)
        {
            return !a.Equals(b);
        }
        public static bool operator ==(Ray a, Ray b)
        {
            return a.Equals(b);
        }
        internal string DebugDisplayString
        {
            get
            {
                return string.Concat(
                    "Поз( ", this.Position.DebugDisplayString, " ) \ г \ п",
                    "Реж.( ", this.Direction.DebugDisplayString, " )"
                );
            }
        }
        public override string ToString()
        {
            return "{{Позиция:" + Position.ToString() + " Direction:" + Direction.ToString() + "}}";
        }
        public void Deconstruct(out Vector3 position, out Vector3 direction)
        {
            position = Position;
            direction = Direction;
        }
        #endregion
    }
}