
using System;
using System.Diagnostics;
namespace Mathf
{
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    public class BoundingFrustum : IEquatable<BoundingFrustum>
    {
        #region Private Fields
        private Matrix _matrix;
        private readonly Vector3[] _corners = new Vector3[CornerCount];
        private readonly Plane[] _planes = new Plane[PlaneCount];
        #endregion
        #region Public Fields
        public const int PlaneCount = 6;
        public const int CornerCount = 8;
        #endregion
        #region Properties
        public Matrix Matrix
        {
            get { return this._matrix; }
            set
            {
                this._matrix = value;
                this.CreatePlanes();
                this.CreateCorners();
            }
        }
        public Plane Near
        {
            get { return this._planes[0]; }
        }
        public Plane Far
        {
            get { return this._planes[1]; }
        }
        public Plane Left
        {
            get { return this._planes[2]; }
        }
        public Plane Right
        {
            get { return this._planes[3]; }
        }
        public Plane Top
        {
            get { return this._planes[4]; }
        }
        public Plane Bottom
        {
            get { return this._planes[5]; }
        }
        #endregion
        #region Internal Properties
        internal string DebugDisplayString
        {
            get
            {
                return string.Concat(
                    "Около( ", this._planes[0].DebugDisplayString, " ) \ г \ п",
                    "Далеко( ", this._planes[1].DebugDisplayString, " ) \ г \ п",
                    "Левый( ", this._planes[2].DebugDisplayString, " ) \ г \ п",
                    "Верно( ", this._planes[3].DebugDisplayString, " ) \ г \ п",
                    "Вершина( ", this._planes[4].DebugDisplayString, " ) \ г \ п",
                    "Нижний( ", this._planes[5].DebugDisplayString, " )  "
                    );
            }
        }
        #endregion
        #region Constructors
        public BoundingFrustum(Matrix value)
        {
            this._matrix = value;
            this.CreatePlanes();
            this.CreateCorners();
        }
        #endregion
        #region Operators
        public static bool operator ==(BoundingFrustum a, BoundingFrustum b)
        {
            if (Equals(a, null))
                return (Equals(b, null));
            if (Equals(b, null))
                return (Equals(a, null));
            return a._matrix == (b._matrix);
        }
        public static bool operator !=(BoundingFrustum a, BoundingFrustum b)
        {
            return !(a == b);
        }
        #endregion
        #region Public Methods
        #region Contains
        public ContainmentType Contains(BoundingBox box)
        {
            var result = default(ContainmentType);
            this.Contains(ref box, out result);
            return result;
        }
        public void Contains(ref BoundingBox box, out ContainmentType result)
        {
            var intersects = false;
            for (var i = 0; i < PlaneCount; ++i)
            {
                var planeIntersectionType = default(PlaneIntersectionType);
                box.Intersects(ref this._planes[i], out planeIntersectionType);
                switch (planeIntersectionType)
                {
                case PlaneIntersectionType.Front:
                    result = ContainmentType.Disjoint;
                    return;
                case PlaneIntersectionType.Intersecting:
                    intersects = true;
                    break;
                }
            }
            result = intersects ? ContainmentType.Intersects : ContainmentType.Contains;
        }
        public ContainmentType Contains(BoundingFrustum frustum)
        {
            if (this == frustum)
                return ContainmentType.Contains;
            var intersects = false;
            for (var i = 0; i < PlaneCount; ++i)
            {
                PlaneIntersectionType planeIntersectionType;
                frustum.Intersects(ref _planes[i], out planeIntersectionType);
                switch (planeIntersectionType)
                {
                    case PlaneIntersectionType.Front:
                        return ContainmentType.Disjoint;
                    case PlaneIntersectionType.Intersecting:
                        intersects = true;
                        break;
                }
            }
            return intersects ? ContainmentType.Intersects : ContainmentType.Contains;
        }
        public ContainmentType Contains(BoundingSphere sphere)
        {
            var result = default(ContainmentType);
            this.Contains(ref sphere, out result);
            return result;
        }
        public void Contains(ref BoundingSphere sphere, out ContainmentType result)
        {
            var intersects = false;
            for (var i = 0; i < PlaneCount; ++i)
            {
                var planeIntersectionType = default(PlaneIntersectionType);
                sphere.Intersects(ref this._planes[i], out planeIntersectionType);
                switch (planeIntersectionType)
                {
                case PlaneIntersectionType.Front:
                    result = ContainmentType.Disjoint;
                    return;
                case PlaneIntersectionType.Intersecting:
                    intersects = true;
                    break;
                }
            }
            result = intersects ? ContainmentType.Intersects : ContainmentType.Contains;
        }
        public ContainmentType Contains(Vector3 point)
        {
            var result = default(ContainmentType);
            this.Contains(ref point, out result);
            return result;
        }
        public void Contains(ref Vector3 point, out ContainmentType result)
        {
            for (var i = 0; i < PlaneCount; ++i)
            {
                if (PlaneHelper.ClassifyPoint(ref point, ref this._planes[i]) > 0)
                {
                    result = ContainmentType.Disjoint;
                    return;
                }
            }
            result = ContainmentType.Contains;
        }
        #endregion
        public bool Equals(BoundingFrustum other)
        {
            return (this == other);
        }
        public override bool Equals(object obj)
        {
            return (obj is BoundingFrustum) && this == ((BoundingFrustum)obj);
        }
        public Vector3[] GetCorners()
        {
            return (Vector3[])this._corners.Clone();
        }
		public void GetCorners(Vector3[] corners)
        {
			if (corners == null) throw new ArgumentNullException("corners");
		    if (corners.Length < CornerCount) throw new ArgumentOutOfRangeException("corners");
            this._corners.CopyTo(corners, 0);
        }
        public override int GetHashCode()
        {
            return this._matrix.GetHashCode();
        }
        public bool Intersects(BoundingBox box)
        {
			var result = false;
			this.Intersects(ref box, out result);
			return result;
        }
        public void Intersects(ref BoundingBox box, out bool result)
        {
			var containment = default(ContainmentType);
			this.Contains(ref box, out containment);
			result = containment != ContainmentType.Disjoint;
		}
        public bool Intersects(BoundingFrustum frustum)
        {
            return Contains(frustum) != ContainmentType.Disjoint;
        }
        public bool Intersects(BoundingSphere sphere)
        {
            var result = default(bool);
            this.Intersects(ref sphere, out result);
            return result;
        }
        public void Intersects(ref BoundingSphere sphere, out bool result)
        {
            var containment = default(ContainmentType);
            this.Contains(ref sphere, out containment);
            result = containment != ContainmentType.Disjoint;
        }
        public PlaneIntersectionType Intersects(Plane plane)
        {
            PlaneIntersectionType result;
            Intersects(ref plane, out result);
            return result;
        }
        public void Intersects(ref Plane plane, out PlaneIntersectionType result)
        {
            result = plane.Intersects(ref _corners[0]);
            for (int i = 1; i < _corners.Length; i++)
                if (plane.Intersects(ref _corners[i]) != result)
                    result = PlaneIntersectionType.Intersecting;
        }
        public float? Intersects(Ray ray)
        {
            float? result;
            Intersects(ref ray, out result);
            return result;
        }
        public void Intersects(ref Ray ray, out float? result)
        {
            ContainmentType ctype;
            this.Contains(ref ray.Position, out ctype);
            switch (ctype)
            {
                case ContainmentType.Disjoint:
                    result = null;
                    return;
                case ContainmentType.Contains:
                    result = 0.0f;
                    return;
                case ContainmentType.Intersects:
                    throw new NotImplementedException();
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
        public override string ToString()
        {
            return "{Около: " + this._planes[0] +
                   " Far:" + this._planes[1] +
                   " Left:" + this._planes[2] +
                   " Right:" + this._planes[3] +
                   " Top:" + this._planes[4] +
                   " Bottom:" + this._planes[5] +
                   "}";
        }
        #endregion
        #region Private Methods
        private void CreateCorners()
        {
            IntersectionPoint(ref this._planes[0], ref this._planes[2], ref this._planes[4], out this._corners[0]);
            IntersectionPoint(ref this._planes[0], ref this._planes[3], ref this._planes[4], out this._corners[1]);
            IntersectionPoint(ref this._planes[0], ref this._planes[3], ref this._planes[5], out this._corners[2]);
            IntersectionPoint(ref this._planes[0], ref this._planes[2], ref this._planes[5], out this._corners[3]);
            IntersectionPoint(ref this._planes[1], ref this._planes[2], ref this._planes[4], out this._corners[4]);
            IntersectionPoint(ref this._planes[1], ref this._planes[3], ref this._planes[4], out this._corners[5]);
            IntersectionPoint(ref this._planes[1], ref this._planes[3], ref this._planes[5], out this._corners[6]);
            IntersectionPoint(ref this._planes[1], ref this._planes[2], ref this._planes[5], out this._corners[7]);
        }
        private void CreatePlanes()
        {
            this._planes[0] = new Plane(-this._matrix.M13, -this._matrix.M23, -this._matrix.M33, -this._matrix.M43);
            this._planes[1] = new Plane(this._matrix.M13 - this._matrix.M14, this._matrix.M23 - this._matrix.M24, this._matrix.M33 - this._matrix.M34, this._matrix.M43 - this._matrix.M44);
            this._planes[2] = new Plane(-this._matrix.M14 - this._matrix.M11, -this._matrix.M24 - this._matrix.M21, -this._matrix.M34 - this._matrix.M31, -this._matrix.M44 - this._matrix.M41);
            this._planes[3] = new Plane(this._matrix.M11 - this._matrix.M14, this._matrix.M21 - this._matrix.M24, this._matrix.M31 - this._matrix.M34, this._matrix.M41 - this._matrix.M44);
            this._planes[4] = new Plane(this._matrix.M12 - this._matrix.M14, this._matrix.M22 - this._matrix.M24, this._matrix.M32 - this._matrix.M34, this._matrix.M42 - this._matrix.M44);
            this._planes[5] = new Plane(-this._matrix.M14 - this._matrix.M12, -this._matrix.M24 - this._matrix.M22, -this._matrix.M34 - this._matrix.M32, -this._matrix.M44 - this._matrix.M42);
            this.NormalizePlane(ref this._planes[0]);
            this.NormalizePlane(ref this._planes[1]);
            this.NormalizePlane(ref this._planes[2]);
            this.NormalizePlane(ref this._planes[3]);
            this.NormalizePlane(ref this._planes[4]);
            this.NormalizePlane(ref this._planes[5]);
        }
        private static void IntersectionPoint(ref Plane a, ref Plane b, ref Plane c, out Vector3 result)
        {
            Vector3 v1, v2, v3;
            Vector3 cross;
            Vector3.Cross(ref b.Normal, ref c.Normal, out cross);
            float f;
            Vector3.Dot(ref a.Normal, ref cross, out f);
            f *= -1.0f;
            Vector3.Cross(ref b.Normal, ref c.Normal, out cross);
            Vector3.Multiply(ref cross, a.D, out v1);
            Vector3.Cross(ref c.Normal, ref a.Normal, out cross);
            Vector3.Multiply(ref cross, b.D, out v2);
            Vector3.Cross(ref a.Normal, ref b.Normal, out cross);
            Vector3.Multiply(ref cross, c.D, out v3);
            result.X = (v1.X + v2.X + v3.X) / f;
            result.Y = (v1.Y + v2.Y + v3.Y) / f;
            result.Z = (v1.Z + v2.Z + v3.Z) / f;
        }
        private void NormalizePlane(ref Plane p)
        {
            float factor = 1f / p.Normal.Length();
            p.Normal.X *= factor;
            p.Normal.Y *= factor;
            p.Normal.Z *= factor;
            p.D *= factor;
        }
        #endregion
    }
}