using System;
using System.Runtime.Serialization;
using System.Diagnostics;
using System.Runtime.InteropServices;
namespace Mathf
{
#if XNADESIGNPROVIDED
    [System.ComponentModel.TypeConverter(typeof(Microsoft.Xna.Framework.Design.Vector4TypeConverter))]
#endif
    [DataContract]
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    [StructLayout(LayoutKind.Sequential, Pack = 16, Size = 16)]
    public struct Vector4 : IEquatable<Vector4>
    {
        #region Private Fields
        private static readonly Vector4 zero = new Vector4();
        private static readonly Vector4 one = new Vector4(1f, 1f, 1f, 1f);
        private static readonly Vector4 unitX = new Vector4(1f, 0f, 0f, 0f);
        private static readonly Vector4 unitY = new Vector4(0f, 1f, 0f, 0f);
        private static readonly Vector4 unitZ = new Vector4(0f, 0f, 1f, 0f);
        private static readonly Vector4 unitW = new Vector4(0f, 0f, 0f, 1f);
        #endregion
        #region Public Fields
        [DataMember]
        public float X;
        [DataMember]
        public float Y;
        [DataMember]
        public float Z;
        [DataMember]
        public float W;
        #endregion
        #region Public Properties
        public static Vector4 Zero
        {
            get { return zero; }
        }
        public static Vector4 One
        {
            get { return one; }
        }
        public static Vector4 UnitX
        {
            get { return unitX; }
        }
        public static Vector4 UnitY
        {
            get { return unitY; }
        }
        public static Vector4 UnitZ
        {
            get { return unitZ; }
        }
        public static Vector4 UnitW
        {
            get { return unitW; }
        }
        #endregion
        #region Internal Properties
        internal string DebugDisplayString
        {
            get
            {
                return string.Concat(
                    this.X.ToString(), "  ",
                    this.Y.ToString(), "  ",
                    this.Z.ToString(), "  ",
                    this.W.ToString()
                );
            }
        }
        #endregion
        #region Constructors
        public Vector4(float x, float y, float z, float w)
        {
            this.X = x;
            this.Y = y;
            this.Z = z;
            this.W = w;
        }
        public Vector4(Vector2 value, float z, float w)
        {
            this.X = value.X;
            this.Y = value.Y;
            this.Z = z;
            this.W = w;
        }
        public Vector4(Vector3 value, float w)
        {
            this.X = value.X;
            this.Y = value.Y;
            this.Z = value.Z;
            this.W = w;
        }
        public Vector4(float value)
        {
            this.X = value;
            this.Y = value;
            this.Z = value;
            this.W = value;
        }
        #endregion
        #region Public Methods
        public static Vector4 Add(Vector4 value1, Vector4 value2)
        {
            value1.X += value2.X;
            value1.Y += value2.Y;
            value1.Z += value2.Z;
            value1.W += value2.W;
            return value1;
        }
        public static void Add(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.X = value1.X + value2.X;
            result.Y = value1.Y + value2.Y;
            result.Z = value1.Z + value2.Z;
            result.W = value1.W + value2.W;
        }
        public static Vector4 Barycentric(Vector4 value1, Vector4 value2, Vector4 value3, float amount1, float amount2)
        {
            return new Vector4(
                MathHelper.Barycentric(value1.X, value2.X, value3.X, amount1, amount2),
                MathHelper.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2),
                MathHelper.Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2),
                MathHelper.Barycentric(value1.W, value2.W, value3.W, amount1, amount2));
        }
        public static void Barycentric(ref Vector4 value1, ref Vector4 value2, ref Vector4 value3, float amount1, float amount2, out Vector4 result)
        {
            result.X = MathHelper.Barycentric(value1.X, value2.X, value3.X, amount1, amount2);
            result.Y = MathHelper.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2);
            result.Z = MathHelper.Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2);
            result.W = MathHelper.Barycentric(value1.W, value2.W, value3.W, amount1, amount2);
        }
        public static Vector4 CatmullRom(Vector4 value1, Vector4 value2, Vector4 value3, Vector4 value4, float amount)
        {
            return new Vector4(
                MathHelper.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount),
                MathHelper.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount),
                MathHelper.CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount),
                MathHelper.CatmullRom(value1.W, value2.W, value3.W, value4.W, amount));
        }
        public static void CatmullRom(ref Vector4 value1, ref Vector4 value2, ref Vector4 value3, ref Vector4 value4, float amount, out Vector4 result)
        {
            result.X = MathHelper.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount);
            result.Y = MathHelper.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount);
            result.Z = MathHelper.CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount);
            result.W = MathHelper.CatmullRom(value1.W, value2.W, value3.W, value4.W, amount);
        }
        public void Ceiling()
        {
            X = (float)Math.Ceiling(X);
            Y = (float)Math.Ceiling(Y);
            Z = (float)Math.Ceiling(Z);
            W = (float)Math.Ceiling(W);
        }
        public static Vector4 Ceiling(Vector4 value)
        {
            value.X = (float)Math.Ceiling(value.X);
            value.Y = (float)Math.Ceiling(value.Y);
            value.Z = (float)Math.Ceiling(value.Z);
            value.W = (float)Math.Ceiling(value.W);
            return value;
        }
        public static void Ceiling(ref Vector4 value, out Vector4 result)
        {
            result.X = (float)Math.Ceiling(value.X);
            result.Y = (float)Math.Ceiling(value.Y);
            result.Z = (float)Math.Ceiling(value.Z);
            result.W = (float)Math.Ceiling(value.W);
        }
        public static Vector4 Clamp(Vector4 value1, Vector4 min, Vector4 max)
        {
            return new Vector4(
                MathHelper.Clamp(value1.X, min.X, max.X),
                MathHelper.Clamp(value1.Y, min.Y, max.Y),
                MathHelper.Clamp(value1.Z, min.Z, max.Z),
                MathHelper.Clamp(value1.W, min.W, max.W));
        }
        public static void Clamp(ref Vector4 value1, ref Vector4 min, ref Vector4 max, out Vector4 result)
        {
            result.X = MathHelper.Clamp(value1.X, min.X, max.X);
            result.Y = MathHelper.Clamp(value1.Y, min.Y, max.Y);
            result.Z = MathHelper.Clamp(value1.Z, min.Z, max.Z);
            result.W = MathHelper.Clamp(value1.W, min.W, max.W);
        }
        public static float Distance(Vector4 value1, Vector4 value2)
        {
            return (float)Math.Sqrt(DistanceSquared(value1, value2));
        }
        public static void Distance(ref Vector4 value1, ref Vector4 value2, out float result)
        {
            result = (float)Math.Sqrt(DistanceSquared(value1, value2));
        }
        public static float DistanceSquared(Vector4 value1, Vector4 value2)
        {
              return (value1.W - value2.W) * (value1.W - value2.W) +
                     (value1.X - value2.X) * (value1.X - value2.X) +
                     (value1.Y - value2.Y) * (value1.Y - value2.Y) +
                     (value1.Z - value2.Z) * (value1.Z - value2.Z);
        }
        public static void DistanceSquared(ref Vector4 value1, ref Vector4 value2, out float result)
        {
            result = (value1.W - value2.W) * (value1.W - value2.W) +
                     (value1.X - value2.X) * (value1.X - value2.X) +
                     (value1.Y - value2.Y) * (value1.Y - value2.Y) +
                     (value1.Z - value2.Z) * (value1.Z - value2.Z);
        }
        public static Vector4 Divide(Vector4 value1, Vector4 value2)
        {
            value1.W /= value2.W;
            value1.X /= value2.X;
            value1.Y /= value2.Y;
            value1.Z /= value2.Z;
            return value1;
        }
        public static Vector4 Divide(Vector4 value1, float divider)
        {
            float factor = 1f / divider;
            value1.W *= factor;
            value1.X *= factor;
            value1.Y *= factor;
            value1.Z *= factor;
            return value1;
        }
        public static void Divide(ref Vector4 value1, float divider, out Vector4 result)
        {
            float factor = 1f / divider;
            result.W = value1.W * factor;
            result.X = value1.X * factor;
            result.Y = value1.Y * factor;
            result.Z = value1.Z * factor;
        }
        public static void Divide(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.W = value1.W / value2.W;
            result.X = value1.X / value2.X;
            result.Y = value1.Y / value2.Y;
            result.Z = value1.Z / value2.Z;
        }
        public static float Dot(Vector4 value1, Vector4 value2)
        {
            return value1.X * value2.X + value1.Y * value2.Y + value1.Z * value2.Z + value1.W * value2.W;
        }
        public static void Dot(ref Vector4 value1, ref Vector4 value2, out float result)
        {
            result = value1.X * value2.X + value1.Y * value2.Y + value1.Z * value2.Z + value1.W * value2.W;
        }
        public override bool Equals(object obj)
        {
            return (obj is Vector4) ? this == (Vector4)obj : false;
        }
        public bool Equals(Vector4 other)
        {
            return this.W == other.W
                && this.X == other.X
                && this.Y == other.Y
                && this.Z == other.Z;
        }
        public void Floor()
        {
            X = (float)Math.Floor(X);
            Y = (float)Math.Floor(Y);
            Z = (float)Math.Floor(Z);
            W = (float)Math.Floor(W);
        }
        public static Vector4 Floor(Vector4 value)
        {
            value.X = (float)Math.Floor(value.X);
            value.Y = (float)Math.Floor(value.Y);
            value.Z = (float)Math.Floor(value.Z);
            value.W = (float)Math.Floor(value.W);
            return value;
        }
        public static void Floor(ref Vector4 value, out Vector4 result)
        {
            result.X = (float)Math.Floor(value.X);
            result.Y = (float)Math.Floor(value.Y);
            result.Z = (float)Math.Floor(value.Z);
            result.W = (float)Math.Floor(value.W);
        }
        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = W.GetHashCode();
                hashCode = (hashCode * 397) ^ X.GetHashCode();
                hashCode = (hashCode * 397) ^ Y.GetHashCode();
                hashCode = (hashCode * 397) ^ Z.GetHashCode();
                return hashCode;
            }
        }
        public static Vector4 Hermite(Vector4 value1, Vector4 tangent1, Vector4 value2, Vector4 tangent2, float amount)
        {
            return new Vector4(MathHelper.Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount),
                               MathHelper.Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount),
                               MathHelper.Hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, amount),
                               MathHelper.Hermite(value1.W, tangent1.W, value2.W, tangent2.W, amount));
        }
        public static void Hermite(ref Vector4 value1, ref Vector4 tangent1, ref Vector4 value2, ref Vector4 tangent2, float amount, out Vector4 result)
        {
            result.W = MathHelper.Hermite(value1.W, tangent1.W, value2.W, tangent2.W, amount);
            result.X = MathHelper.Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount);
            result.Y = MathHelper.Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount);
            result.Z = MathHelper.Hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, amount);
        }
        public float Length()
        {
            return (float)Math.Sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
        }
        public float LengthSquared()
        {
            return (X * X) + (Y * Y) + (Z * Z) + (W * W);
        }
        public static Vector4 Lerp(Vector4 value1, Vector4 value2, float amount)
        {
            return new Vector4(
                MathHelper.Lerp(value1.X, value2.X, amount),
                MathHelper.Lerp(value1.Y, value2.Y, amount),
                MathHelper.Lerp(value1.Z, value2.Z, amount),
                MathHelper.Lerp(value1.W, value2.W, amount));
        }
        public static void Lerp(ref Vector4 value1, ref Vector4 value2, float amount, out Vector4 result)
        {
            result.X = MathHelper.Lerp(value1.X, value2.X, amount);
            result.Y = MathHelper.Lerp(value1.Y, value2.Y, amount);
            result.Z = MathHelper.Lerp(value1.Z, value2.Z, amount);
            result.W = MathHelper.Lerp(value1.W, value2.W, amount);
        }
        public static Vector4 LerpPrecise(Vector4 value1, Vector4 value2, float amount)
        {
            return new Vector4(
                MathHelper.LerpPrecise(value1.X, value2.X, amount),
                MathHelper.LerpPrecise(value1.Y, value2.Y, amount),
                MathHelper.LerpPrecise(value1.Z, value2.Z, amount),
                MathHelper.LerpPrecise(value1.W, value2.W, amount));
        }
        public static void LerpPrecise(ref Vector4 value1, ref Vector4 value2, float amount, out Vector4 result)
        {
            result.X = MathHelper.LerpPrecise(value1.X, value2.X, amount);
            result.Y = MathHelper.LerpPrecise(value1.Y, value2.Y, amount);
            result.Z = MathHelper.LerpPrecise(value1.Z, value2.Z, amount);
            result.W = MathHelper.LerpPrecise(value1.W, value2.W, amount);
        }
        public static Vector4 Max(Vector4 value1, Vector4 value2)
        {
            return new Vector4(
               MathHelper.Max(value1.X, value2.X),
               MathHelper.Max(value1.Y, value2.Y),
               MathHelper.Max(value1.Z, value2.Z),
               MathHelper.Max(value1.W, value2.W));
        }
        public static void Max(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.X = MathHelper.Max(value1.X, value2.X);
            result.Y = MathHelper.Max(value1.Y, value2.Y);
            result.Z = MathHelper.Max(value1.Z, value2.Z);
            result.W = MathHelper.Max(value1.W, value2.W);
        }
        public static Vector4 Min(Vector4 value1, Vector4 value2)
        {
            return new Vector4(
               MathHelper.Min(value1.X, value2.X),
               MathHelper.Min(value1.Y, value2.Y),
               MathHelper.Min(value1.Z, value2.Z),
               MathHelper.Min(value1.W, value2.W));
        }
        public static void Min(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.X = MathHelper.Min(value1.X, value2.X);
            result.Y = MathHelper.Min(value1.Y, value2.Y);
            result.Z = MathHelper.Min(value1.Z, value2.Z);
            result.W = MathHelper.Min(value1.W, value2.W);
        }
        public static Vector4 Multiply(Vector4 value1, Vector4 value2)
        {
            value1.W *= value2.W;
            value1.X *= value2.X;
            value1.Y *= value2.Y;
            value1.Z *= value2.Z;
            return value1;
        }
        public static Vector4 Multiply(Vector4 value1, float scaleFactor)
        {
            value1.W *= scaleFactor;
            value1.X *= scaleFactor;
            value1.Y *= scaleFactor;
            value1.Z *= scaleFactor;
            return value1;
        }
        public static void Multiply(ref Vector4 value1, float scaleFactor, out Vector4 result)
        {
            result.W = value1.W * scaleFactor;
            result.X = value1.X * scaleFactor;
            result.Y = value1.Y * scaleFactor;
            result.Z = value1.Z * scaleFactor;
        }
        public static void Multiply(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.W = value1.W * value2.W;
            result.X = value1.X * value2.X;
            result.Y = value1.Y * value2.Y;
            result.Z = value1.Z * value2.Z;
        }
        public static Vector4 Negate(Vector4 value)
        {
            value = new Vector4(-value.X, -value.Y, -value.Z, -value.W);
            return value;
        }
        public static void Negate(ref Vector4 value, out Vector4 result)
        {
            result.X = -value.X;
            result.Y = -value.Y;
            result.Z = -value.Z;
            result.W = -value.W;
        }
        public void Normalize()
        {
            float factor = (float)Math.Sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
            factor = 1f / factor;
            X *= factor;
            Y *= factor;
            Z *= factor;
            W *= factor;
        }
        public static Vector4 Normalize(Vector4 value)
        {
            float factor = (float)Math.Sqrt((value.X * value.X) + (value.Y * value.Y) + (value.Z * value.Z) + (value.W * value.W));
            factor = 1f / factor;
            return new Vector4(value.X*factor,value.Y*factor,value.Z*factor,value.W*factor);
        }
        public static void Normalize(ref Vector4 value, out Vector4 result)
        {
            float factor = (float)Math.Sqrt((value.X * value.X) + (value.Y * value.Y) + (value.Z * value.Z) + (value.W * value.W));
            factor = 1f / factor;
            result.W = value.W * factor;
            result.X = value.X * factor;
            result.Y = value.Y * factor;
            result.Z = value.Z * factor;
        }
        public void Round()
        {
            X = (float)Math.Round(X);
            Y = (float)Math.Round(Y);
            Z = (float)Math.Round(Z);
            W = (float)Math.Round(W);
        }
        public static Vector4 Round(Vector4 value)
        {
            value.X = (float)Math.Round(value.X);
            value.Y = (float)Math.Round(value.Y);
            value.Z = (float)Math.Round(value.Z);
            value.W = (float)Math.Round(value.W);
            return value;
        }
        public static void Round(ref Vector4 value, out Vector4 result)
        {
            result.X = (float)Math.Round(value.X);
            result.Y = (float)Math.Round(value.Y);
            result.Z = (float)Math.Round(value.Z);
            result.W = (float)Math.Round(value.W);
        }
        public static Vector4 SmoothStep(Vector4 value1, Vector4 value2, float amount)
        {
            return new Vector4(
                MathHelper.SmoothStep(value1.X, value2.X, amount),
                MathHelper.SmoothStep(value1.Y, value2.Y, amount),
                MathHelper.SmoothStep(value1.Z, value2.Z, amount),
                MathHelper.SmoothStep(value1.W, value2.W, amount));
        }
        public static void SmoothStep(ref Vector4 value1, ref Vector4 value2, float amount, out Vector4 result)
        {
            result.X = MathHelper.SmoothStep(value1.X, value2.X, amount);
            result.Y = MathHelper.SmoothStep(value1.Y, value2.Y, amount);
            result.Z = MathHelper.SmoothStep(value1.Z, value2.Z, amount);
            result.W = MathHelper.SmoothStep(value1.W, value2.W, amount);
        }
        public static Vector4 Subtract(Vector4 value1, Vector4 value2)
        {
            value1.W -= value2.W;
            value1.X -= value2.X;
            value1.Y -= value2.Y;
            value1.Z -= value2.Z;
            return value1;
        }
        public static void Subtract(ref Vector4 value1, ref Vector4 value2, out Vector4 result)
        {
            result.W = value1.W - value2.W;
            result.X = value1.X - value2.X;
            result.Y = value1.Y - value2.Y;
            result.Z = value1.Z - value2.Z;
        }
        #region Transform
        public static Vector4 Transform(Vector2 value, Matrix matrix)
        {
            Vector4 result;
            Transform(ref value, ref matrix, out result);
            return result;
        }
        public static Vector4 Transform(Vector2 value, Quaternion rotation)
        {
            Vector4 result;
            Transform(ref value, ref rotation, out result);
            return result;
        }
        public static Vector4 Transform(Vector3 value, Matrix matrix)
        {
            Vector4 result;
            Transform(ref value, ref matrix, out result);
            return result;
        }
        public static Vector4 Transform(Vector3 value, Quaternion rotation)
        {
            Vector4 result;
            Transform(ref value, ref rotation, out result);
            return result;
        }
        public static Vector4 Transform(Vector4 value, Matrix matrix)
        {
            Transform(ref value, ref matrix, out value);
            return value;
        }
        public static Vector4 Transform(Vector4 value, Quaternion rotation)
        {
            Vector4 result;
            Transform(ref value, ref rotation, out result);
            return result;
        }
        public static void Transform(ref Vector2 value, ref Matrix matrix, out Vector4 result)
        {
            result.X = (value.X * matrix.M11) + (value.Y * matrix.M21) + matrix.M41;
            result.Y = (value.X * matrix.M12) + (value.Y * matrix.M22) + matrix.M42;
            result.Z = (value.X * matrix.M13) + (value.Y * matrix.M23) + matrix.M43;
            result.W = (value.X * matrix.M14) + (value.Y * matrix.M24) + matrix.M44;
        }
        public static void Transform(ref Vector2 value, ref Quaternion rotation, out Vector4 result)
        {
            throw new NotImplementedException();
        }
        public static void Transform(ref Vector3 value, ref Matrix matrix, out Vector4 result)
        {
            result.X = (value.X * matrix.M11) + (value.Y * matrix.M21) + (value.Z * matrix.M31) + matrix.M41;
            result.Y = (value.X * matrix.M12) + (value.Y * matrix.M22) + (value.Z * matrix.M32) + matrix.M42;
            result.Z = (value.X * matrix.M13) + (value.Y * matrix.M23) + (value.Z * matrix.M33) + matrix.M43;
            result.W = (value.X * matrix.M14) + (value.Y * matrix.M24) + (value.Z * matrix.M34) + matrix.M44;
        }
        public static void Transform(ref Vector3 value, ref Quaternion rotation, out Vector4 result)
        {
            throw new NotImplementedException();
        }
        public static void Transform(ref Vector4 value, ref Matrix matrix, out Vector4 result)
        {
            var x = (value.X * matrix.M11) + (value.Y * matrix.M21) + (value.Z * matrix.M31) + (value.W * matrix.M41);
            var y = (value.X * matrix.M12) + (value.Y * matrix.M22) + (value.Z * matrix.M32) + (value.W * matrix.M42);
            var z = (value.X * matrix.M13) + (value.Y * matrix.M23) + (value.Z * matrix.M33) + (value.W * matrix.M43);
            var w = (value.X * matrix.M14) + (value.Y * matrix.M24) + (value.Z * matrix.M34) + (value.W * matrix.M44);
            result.X = x;
            result.Y = y;
            result.Z = z;
            result.W = w;
        }
        public static void Transform(ref Vector4 value, ref Quaternion rotation, out Vector4 result)
        {
            throw new NotImplementedException();
        }
        public static void Transform
        (
            Vector4[] sourceArray,
            int sourceIndex,
            ref Matrix matrix,
            Vector4[] destinationArray,
            int destinationIndex,
            int length
        )
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (sourceArray.Length < sourceIndex + length)
                throw new ArgumentException("Длина исходного массива меньше, чем sourceIndex + длина");
            if (destinationArray.Length < destinationIndex + length)
                throw new ArgumentException("Длина целевого массива меньше, чем DestinationIndex + длина.");
            for (var i = 0; i < length; i++)
            {
                var value = sourceArray[sourceIndex + i];
                destinationArray[destinationIndex + i] = Transform(value, matrix);
            }
        }
        public static void Transform(
            Vector4[] sourceArray,
            int sourceIndex,
            ref Quaternion rotation,
            Vector4[] destinationArray,
            int destinationIndex,
            int length
            )
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (sourceArray.Length < sourceIndex + length)
                throw new ArgumentException("Длина исходного массива меньше, чем sourceIndex + длина");
            if (destinationArray.Length < destinationIndex + length)
                throw new ArgumentException("Длина целевого массива меньше, чем DestinationIndex + длина.");
            for (var i = 0; i < length; i++)
            {
                var value = sourceArray[sourceIndex + i];
                destinationArray[destinationIndex + i] = Transform(value, rotation);
            }
        }
        public static void Transform(Vector4[] sourceArray, ref Matrix matrix, Vector4[] destinationArray)
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (destinationArray.Length < sourceArray.Length)
                throw new ArgumentException("Длина целевого массива меньше длины исходного массива");
            for (var i = 0; i < sourceArray.Length; i++)
            {
                var value = sourceArray[i];
                destinationArray[i] = Transform(value, matrix);
            }
        }
        public static void Transform(Vector4[] sourceArray, ref Quaternion rotation, Vector4[] destinationArray)
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (destinationArray.Length < sourceArray.Length)
                throw new ArgumentException("Длина целевого массива меньше длины исходного массива");
            for (var i = 0; i < sourceArray.Length; i++)
            {
                var value = sourceArray[i];
                destinationArray[i] = Transform(value, rotation);
            }
        }
        #endregion
        public override string ToString()
        {
            return "{X:" + X + " Y:" + Y + " Z:" + Z + " W:" + W + "}";
        }
        public void Deconstruct(out float x, out float y, out float z, out float w)
        {
            x = X;
            y = Y;
            z = Z;
            w = W;
        }
        #endregion
        #region Operators
        public static Vector4 operator -(Vector4 value)
        {
            return new Vector4(-value.X, -value.Y, -value.Z, -value.W);
        }
        public static bool operator ==(Vector4 value1, Vector4 value2)
        {
            return value1.W == value2.W
                && value1.X == value2.X
                && value1.Y == value2.Y
                && value1.Z == value2.Z;
        }
        public static bool operator !=(Vector4 value1, Vector4 value2)
        {
            return !(value1 == value2);
        }
        public static Vector4 operator +(Vector4 value1, Vector4 value2)
        {
            value1.W += value2.W;
            value1.X += value2.X;
            value1.Y += value2.Y;
            value1.Z += value2.Z;
            return value1;
        }
        public static Vector4 operator -(Vector4 value1, Vector4 value2)
        {
            value1.W -= value2.W;
            value1.X -= value2.X;
            value1.Y -= value2.Y;
            value1.Z -= value2.Z;
            return value1;
        }
        public static Vector4 operator *(Vector4 value1, Vector4 value2)
        {
            value1.W *= value2.W;
            value1.X *= value2.X;
            value1.Y *= value2.Y;
            value1.Z *= value2.Z;
            return value1;
        }
        public static Vector4 operator *(Vector4 value, float scaleFactor)
        {
            value.W *= scaleFactor;
            value.X *= scaleFactor;
            value.Y *= scaleFactor;
            value.Z *= scaleFactor;
            return value;
        }
        public static Vector4 operator *(float scaleFactor, Vector4 value)
        {
            value.W *= scaleFactor;
            value.X *= scaleFactor;
            value.Y *= scaleFactor;
            value.Z *= scaleFactor;
            return value;
        }
        public static Vector4 operator /(Vector4 value1, Vector4 value2)
        {
            value1.W /= value2.W;
            value1.X /= value2.X;
            value1.Y /= value2.Y;
            value1.Z /= value2.Z;
            return value1;
        }
        public static Vector4 operator /(Vector4 value1, float divider)
        {
            float factor = 1f / divider;
            value1.W *= factor;
            value1.X *= factor;
            value1.Y *= factor;
            value1.Z *= factor;
            return value1;
        }
        #endregion
    }
}