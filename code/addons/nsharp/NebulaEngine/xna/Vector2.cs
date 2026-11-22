using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
namespace Mathf
{
#if XNADESIGNPROVIDED
    [System.ComponentModel.TypeConverter(typeof(Microsoft.Xna.Framework.Design.Vector2TypeConverter))]
#endif
    [DataContract]
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 : IEquatable<Vector2>
    {
        #region Private Fields
        private static readonly Vector2 zeroVector = new Vector2(0f, 0f);
        private static readonly Vector2 unitVector = new Vector2(1f, 1f);
        private static readonly Vector2 unitXVector = new Vector2(1f, 0f);
        private static readonly Vector2 unitYVector = new Vector2(0f, 1f);
        #endregion
        #region Public Fields
        [DataMember]
        public float X;
        [DataMember]
        public float Y;
        #endregion
        #region Properties
        public static Vector2 Zero
        {
            get { return zeroVector; }
        }
        public static Vector2 One
        {
            get { return unitVector; }
        }
        public static Vector2 UnitX
        {
            get { return unitXVector; }
        }
        public static Vector2 UnitY
        {
            get { return unitYVector; }
        }
        #endregion
        #region Internal Properties
        internal string DebugDisplayString
        {
            get
            {
                return string.Concat(
                    this.X.ToString(), "  ",
                    this.Y.ToString()
                );
            }
        }
        #endregion
        #region Constructors
        public Vector2(float x, float y)
        {
            this.X = x;
            this.Y = y;
        }
        public Vector2(float value)
        {
            this.X = value;
            this.Y = value;
        }
        #endregion
        #region Operators
        public static Vector2 operator -(Vector2 value)
        {
            value.X = -value.X;
            value.Y = -value.Y;
            return value;
        }
        public static Vector2 operator +(Vector2 value1, Vector2 value2)
        {
            value1.X += value2.X;
            value1.Y += value2.Y;
            return value1;
        }
        public static Vector2 operator -(Vector2 value1, Vector2 value2)
        {
            value1.X -= value2.X;
            value1.Y -= value2.Y;
            return value1;
        }
        public static Vector2 operator *(Vector2 value1, Vector2 value2)
        {
            value1.X *= value2.X;
            value1.Y *= value2.Y;
            return value1;
        }
        public static Vector2 operator *(Vector2 value, float scaleFactor)
        {
            value.X *= scaleFactor;
            value.Y *= scaleFactor;
            return value;
        }
        public static Vector2 operator *(float scaleFactor, Vector2 value)
        {
            value.X *= scaleFactor;
            value.Y *= scaleFactor;
            return value;
        }
        public static Vector2 operator /(Vector2 value1, Vector2 value2)
        {
            value1.X /= value2.X;
            value1.Y /= value2.Y;
            return value1;
        }
        public static Vector2 operator /(Vector2 value1, float divider)
        {
            float factor = 1 / divider;
            value1.X *= factor;
            value1.Y *= factor;
            return value1;
        }
        public static bool operator ==(Vector2 value1, Vector2 value2)
        {
            return value1.X == value2.X && value1.Y == value2.Y;
        }
        public static bool operator !=(Vector2 value1, Vector2 value2)
        {
            return value1.X != value2.X || value1.Y != value2.Y;
        }
        #endregion
        #region Public Methods
        public static Vector2 Add(Vector2 value1, Vector2 value2)
        {
            value1.X += value2.X;
            value1.Y += value2.Y;
            return value1;
        }
        public static void Add(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X + value2.X;
            result.Y = value1.Y + value2.Y;
        }
        public static Vector2 Barycentric(Vector2 value1, Vector2 value2, Vector2 value3, float amount1, float amount2)
        {
            return new Vector2(
                MathHelper.Barycentric(value1.X, value2.X, value3.X, amount1, amount2),
                MathHelper.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2));
        }
        public static void Barycentric(ref Vector2 value1, ref Vector2 value2, ref Vector2 value3, float amount1, float amount2, out Vector2 result)
        {
            result.X = MathHelper.Barycentric(value1.X, value2.X, value3.X, amount1, amount2);
            result.Y = MathHelper.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2);
        }
        public static Vector2 CatmullRom(Vector2 value1, Vector2 value2, Vector2 value3, Vector2 value4, float amount)
        {
            return new Vector2(
                MathHelper.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount),
                MathHelper.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount));
        }
        public static void CatmullRom(ref Vector2 value1, ref Vector2 value2, ref Vector2 value3, ref Vector2 value4, float amount, out Vector2 result)
        {
            result.X = MathHelper.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount);
            result.Y = MathHelper.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount);
        }
        public void Ceiling()
        {
            X = (float)Math.Ceiling(X);
            Y = (float)Math.Ceiling(Y);
        }
        public static Vector2 Ceiling(Vector2 value)
        {
            value.X = (float)Math.Ceiling(value.X);
            value.Y = (float)Math.Ceiling(value.Y);
            return value;
        }
        public static void Ceiling(ref Vector2 value, out Vector2 result)
        {
            result.X = (float)Math.Ceiling(value.X);
            result.Y = (float)Math.Ceiling(value.Y);
        }
        public static Vector2 Clamp(Vector2 value1, Vector2 min, Vector2 max)
        {
            return new Vector2(
                MathHelper.Clamp(value1.X, min.X, max.X),
                MathHelper.Clamp(value1.Y, min.Y, max.Y));
        }
        public static void Clamp(ref Vector2 value1, ref Vector2 min, ref Vector2 max, out Vector2 result)
        {
            result.X = MathHelper.Clamp(value1.X, min.X, max.X);
            result.Y = MathHelper.Clamp(value1.Y, min.Y, max.Y);
        }
        public static float Distance(Vector2 value1, Vector2 value2)
        {
            float v1 = value1.X - value2.X, v2 = value1.Y - value2.Y;
            return (float)Math.Sqrt((v1 * v1) + (v2 * v2));
        }
        public static void Distance(ref Vector2 value1, ref Vector2 value2, out float result)
        {
            float v1 = value1.X - value2.X, v2 = value1.Y - value2.Y;
            result = (float)Math.Sqrt((v1 * v1) + (v2 * v2));
        }
        public static float DistanceSquared(Vector2 value1, Vector2 value2)
        {
            float v1 = value1.X - value2.X, v2 = value1.Y - value2.Y;
            return (v1 * v1) + (v2 * v2);
        }
        public static void DistanceSquared(ref Vector2 value1, ref Vector2 value2, out float result)
        {
            float v1 = value1.X - value2.X, v2 = value1.Y - value2.Y;
            result = (v1 * v1) + (v2 * v2);
        }
        public static Vector2 Divide(Vector2 value1, Vector2 value2)
        {
            value1.X /= value2.X;
            value1.Y /= value2.Y;
            return value1;
        }
        public static void Divide(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X / value2.X;
            result.Y = value1.Y / value2.Y;
        }
        public static Vector2 Divide(Vector2 value1, float divider)
        {
            float factor = 1 / divider;
            value1.X *= factor;
            value1.Y *= factor;
            return value1;
        }
        public static void Divide(ref Vector2 value1, float divider, out Vector2 result)
        {
            float factor = 1 / divider;
            result.X = value1.X * factor;
            result.Y = value1.Y * factor;
        }
        public static float Dot(Vector2 value1, Vector2 value2)
        {
            return (value1.X * value2.X) + (value1.Y * value2.Y);
        }
        public static void Dot(ref Vector2 value1, ref Vector2 value2, out float result)
        {
            result = (value1.X * value2.X) + (value1.Y * value2.Y);
        }
        public override bool Equals(object obj)
        {
            if (obj is Vector2)
            {
                return Equals((Vector2)obj);
            }
            return false;
        }
        public bool Equals(Vector2 other)
        {
            return (X == other.X) && (Y == other.Y);
        }
        public void Floor()
        {
            X = (float)Math.Floor(X);
            Y = (float)Math.Floor(Y);
        }
        public static Vector2 Floor(Vector2 value)
        {
            value.X = (float)Math.Floor(value.X);
            value.Y = (float)Math.Floor(value.Y);
            return value;
        }
        public static void Floor(ref Vector2 value, out Vector2 result)
        {
            result.X = (float)Math.Floor(value.X);
            result.Y = (float)Math.Floor(value.Y);
        }
        public override int GetHashCode()
        {
            unchecked
            {
                return (X.GetHashCode() * 397) ^ Y.GetHashCode();
            }
        }
        public static Vector2 Hermite(Vector2 value1, Vector2 tangent1, Vector2 value2, Vector2 tangent2, float amount)
        {
            return new Vector2(MathHelper.Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount), MathHelper.Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount));
        }
        public static void Hermite(ref Vector2 value1, ref Vector2 tangent1, ref Vector2 value2, ref Vector2 tangent2, float amount, out Vector2 result)
        {
            result.X = MathHelper.Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount);
            result.Y = MathHelper.Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount);
        }
        public float Length()
        {
            return (float)Math.Sqrt((X * X) + (Y * Y));
        }
        public float LengthSquared()
        {
            return (X * X) + (Y * Y);
        }
        public static Vector2 Lerp(Vector2 value1, Vector2 value2, float amount)
        {
            return new Vector2(
                MathHelper.Lerp(value1.X, value2.X, amount),
                MathHelper.Lerp(value1.Y, value2.Y, amount));
        }
        public static void Lerp(ref Vector2 value1, ref Vector2 value2, float amount, out Vector2 result)
        {
            result.X = MathHelper.Lerp(value1.X, value2.X, amount);
            result.Y = MathHelper.Lerp(value1.Y, value2.Y, amount);
        }
        public static Vector2 LerpPrecise(Vector2 value1, Vector2 value2, float amount)
        {
            return new Vector2(
                MathHelper.LerpPrecise(value1.X, value2.X, amount),
                MathHelper.LerpPrecise(value1.Y, value2.Y, amount));
        }
        public static void LerpPrecise(ref Vector2 value1, ref Vector2 value2, float amount, out Vector2 result)
        {
            result.X = MathHelper.LerpPrecise(value1.X, value2.X, amount);
            result.Y = MathHelper.LerpPrecise(value1.Y, value2.Y, amount);
        }
        public static Vector2 Max(Vector2 value1, Vector2 value2)
        {
            return new Vector2(value1.X > value2.X ? value1.X : value2.X,
                               value1.Y > value2.Y ? value1.Y : value2.Y);
        }
        public static void Max(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X > value2.X ? value1.X : value2.X;
            result.Y = value1.Y > value2.Y ? value1.Y : value2.Y;
        }
        public static Vector2 Min(Vector2 value1, Vector2 value2)
        {
            return new Vector2(value1.X < value2.X ? value1.X : value2.X,
                               value1.Y < value2.Y ? value1.Y : value2.Y);
        }
        public static void Min(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X < value2.X ? value1.X : value2.X;
            result.Y = value1.Y < value2.Y ? value1.Y : value2.Y;
        }
        public static Vector2 Multiply(Vector2 value1, Vector2 value2)
        {
            value1.X *= value2.X;
            value1.Y *= value2.Y;
            return value1;
        }
        public static void Multiply(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X * value2.X;
            result.Y = value1.Y * value2.Y;
        }
        public static Vector2 Multiply(Vector2 value1, float scaleFactor)
        {
            value1.X *= scaleFactor;
            value1.Y *= scaleFactor;
            return value1;
        }
        public static void Multiply(ref Vector2 value1, float scaleFactor, out Vector2 result)
        {
            result.X = value1.X * scaleFactor;
            result.Y = value1.Y * scaleFactor;
        }
        public static Vector2 Negate(Vector2 value)
        {
            value.X = -value.X;
            value.Y = -value.Y;
            return value;
        }
        public static void Negate(ref Vector2 value, out Vector2 result)
        {
            result.X = -value.X;
            result.Y = -value.Y;
        }
        public void Normalize()
        {
            float val = 1.0f / (float)Math.Sqrt((X * X) + (Y * Y));
            X *= val;
            Y *= val;
        }
        public static Vector2 Normalize(Vector2 value)
        {
            float val = 1.0f / (float)Math.Sqrt((value.X * value.X) + (value.Y * value.Y));
            value.X *= val;
            value.Y *= val;
            return value;
        }
        public static void Normalize(ref Vector2 value, out Vector2 result)
        {
            float val = 1.0f / (float)Math.Sqrt((value.X * value.X) + (value.Y * value.Y));
            result.X = value.X * val;
            result.Y = value.Y * val;
        }
        public static Vector2 Reflect(Vector2 vector, Vector2 normal)
        {
            Vector2 result;
            float val = 2.0f * ((vector.X * normal.X) + (vector.Y * normal.Y));
            result.X = vector.X - (normal.X * val);
            result.Y = vector.Y - (normal.Y * val);
            return result;
        }
        public static void Reflect(ref Vector2 vector, ref Vector2 normal, out Vector2 result)
        {
            float val = 2.0f * ((vector.X * normal.X) + (vector.Y * normal.Y));
            result.X = vector.X - (normal.X * val);
            result.Y = vector.Y - (normal.Y * val);
        }
        public void Round()
        {
            X = (float)Math.Round(X);
            Y = (float)Math.Round(Y);
        }
        public static Vector2 Round(Vector2 value)
        {
            value.X = (float)Math.Round(value.X);
            value.Y = (float)Math.Round(value.Y);
            return value;
        }
        public static void Round(ref Vector2 value, out Vector2 result)
        {
            result.X = (float)Math.Round(value.X);
            result.Y = (float)Math.Round(value.Y);
        }
        public static Vector2 SmoothStep(Vector2 value1, Vector2 value2, float amount)
        {
            return new Vector2(
                MathHelper.SmoothStep(value1.X, value2.X, amount),
                MathHelper.SmoothStep(value1.Y, value2.Y, amount));
        }
        public static void SmoothStep(ref Vector2 value1, ref Vector2 value2, float amount, out Vector2 result)
        {
            result.X = MathHelper.SmoothStep(value1.X, value2.X, amount);
            result.Y = MathHelper.SmoothStep(value1.Y, value2.Y, amount);
        }
        public static Vector2 Subtract(Vector2 value1, Vector2 value2)
        {
            value1.X -= value2.X;
            value1.Y -= value2.Y;
            return value1;
        }
        public static void Subtract(ref Vector2 value1, ref Vector2 value2, out Vector2 result)
        {
            result.X = value1.X - value2.X;
            result.Y = value1.Y - value2.Y;
        }
        public override string ToString()
        {
            return "{X:" + X + " Y:" + Y + "}";
        }
        public Point ToPoint()
        {
            return new Point((int) X,(int) Y);
        }
        public static Vector2 Transform(Vector2 position, Matrix matrix)
        {
            return new Vector2((position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41, (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42);
        }
        public static void Transform(ref Vector2 position, ref Matrix matrix, out Vector2 result)
        {
            var x = (position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41;
            var y = (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42;
            result.X = x;
            result.Y = y;
        }
        public static Vector2 Transform(Vector2 value, Quaternion rotation)
        {
            Transform(ref value, ref rotation, out value);
            return value;
        }
        public static void Transform(ref Vector2 value, ref Quaternion rotation, out Vector2 result)
        {
            var rot1 = new Vector3(rotation.X + rotation.X, rotation.Y + rotation.Y, rotation.Z + rotation.Z);
            var rot2 = new Vector3(rotation.X, rotation.X, rotation.W);
            var rot3 = new Vector3(1, rotation.Y, rotation.Z);
            var rot4 = rot1*rot2;
            var rot5 = rot1*rot3;
            var v = new Vector2();
            v.X = (float)((double)value.X * (1.0 - (double)rot5.Y - (double)rot5.Z) + (double)value.Y * ((double)rot4.Y - (double)rot4.Z));
            v.Y = (float)((double)value.X * ((double)rot4.Y + (double)rot4.Z) + (double)value.Y * (1.0 - (double)rot4.X - (double)rot5.Z));
            result.X = v.X;
            result.Y = v.Y;
        }
        public static void Transform(
            Vector2[] sourceArray,
            int sourceIndex,
            ref Matrix matrix,
            Vector2[] destinationArray,
            int destinationIndex,
            int length)
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (sourceArray.Length < sourceIndex + length)
                throw new ArgumentException("Длина исходного массива меньше, чем sourceIndex + длина");
            if (destinationArray.Length < destinationIndex + length)
                throw new ArgumentException("Длина целевого массива меньше, чем DestinationIndex + длина.");
            for (int x = 0; x < length; x++)
            {
                var position = sourceArray[sourceIndex + x];
                var destination = destinationArray[destinationIndex + x];
                destination.X = (position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41;
                destination.Y = (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42;
                destinationArray[destinationIndex + x] = destination;
            }
        }
        public static void Transform
        (
            Vector2[] sourceArray,
            int sourceIndex,
            ref Quaternion rotation,
            Vector2[] destinationArray,
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
            for (int x = 0; x < length; x++)
            {
                var position = sourceArray[sourceIndex + x];
                var destination = destinationArray[destinationIndex + x];
                Vector2 v;
                Transform(ref position,ref rotation,out v);
                destination.X = v.X;
                destination.Y = v.Y;
                destinationArray[destinationIndex + x] = destination;
            }
        }
        public static void Transform(
            Vector2[] sourceArray,
            ref Matrix matrix,
            Vector2[] destinationArray)
        {
            Transform(sourceArray, 0, ref matrix, destinationArray, 0, sourceArray.Length);
        }
        public static void Transform
        (
            Vector2[] sourceArray,
            ref Quaternion rotation,
            Vector2[] destinationArray
        )
        {
            Transform(sourceArray, 0, ref rotation, destinationArray, 0, sourceArray.Length);
        }
        public static Vector2 TransformNormal(Vector2 normal, Matrix matrix)
        {
            return new Vector2((normal.X * matrix.M11) + (normal.Y * matrix.M21),(normal.X * matrix.M12) + (normal.Y * matrix.M22));
        }
        public static void TransformNormal(ref Vector2 normal, ref Matrix matrix, out Vector2 result)
        {
            var x = (normal.X * matrix.M11) + (normal.Y * matrix.M21);
            var y = (normal.X * matrix.M12) + (normal.Y * matrix.M22);
            result.X = x;
            result.Y = y;
        }
        public static void TransformNormal
        (
            Vector2[] sourceArray,
            int sourceIndex,
            ref Matrix matrix,
            Vector2[] destinationArray,
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
            for (int i = 0; i < length; i++)
            {
                var normal = sourceArray[sourceIndex + i];
                destinationArray[destinationIndex + i] = new Vector2((normal.X * matrix.M11) + (normal.Y * matrix.M21),
                                                                     (normal.X * matrix.M12) + (normal.Y * matrix.M22));
            }
        }
        public static void TransformNormal
            (
            Vector2[] sourceArray,
            ref Matrix matrix,
            Vector2[] destinationArray
            )
        {
            if (sourceArray == null)
                throw new ArgumentNullException("sourceArray");
            if (destinationArray == null)
                throw new ArgumentNullException("destinationArray");
            if (destinationArray.Length < sourceArray.Length)
                throw new ArgumentException("Длина целевого массива меньше длины исходного массива");
            for (int i = 0; i < sourceArray.Length; i++)
            {
                var normal = sourceArray[i];
                destinationArray[i] = new Vector2((normal.X * matrix.M11) + (normal.Y * matrix.M21),
                                                  (normal.X * matrix.M12) + (normal.Y * matrix.M22));
            }
        }
        public void Deconstruct(out float x, out float y)
        {
            x = X;
            y = Y;
        }
        #endregion
    }
}