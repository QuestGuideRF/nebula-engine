using System;
using System.Diagnostics;
using System.Runtime.Serialization;
namespace Mathf
{
    [DataContract]
    [DebuggerDisplay("{DebugDisplayString,nq}")]
    public struct Point : IEquatable<Point>
    {
        #region Private Fields
        private static readonly Point zeroPoint = new Point();
        #endregion
        #region Public Fields
        [DataMember]
        public int X;
        [DataMember]
        public int Y;
        #endregion
        #region Properties
        public static Point Zero
        {
            get { return zeroPoint; }
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
        public Point(int x, int y)
        {
            this.X = x;
            this.Y = y;
        }
        public Point(int value)
        {
            this.X = value;
            this.Y = value;
        }
        #endregion
        #region Operators
        public static Point operator +(Point value1, Point value2)
        {
            return new Point(value1.X + value2.X, value1.Y + value2.Y);
        }
        public static Point operator -(Point value1, Point value2)
        {
            return new Point(value1.X - value2.X, value1.Y - value2.Y);
        }
        public static Point operator *(Point value1, Point value2)
        {
            return new Point(value1.X * value2.X, value1.Y * value2.Y);
        }
        public static Point operator /(Point source, Point divisor)
        {
            return new Point(source.X / divisor.X, source.Y / divisor.Y);
        }
        public static bool operator ==(Point a, Point b)
        {
            return a.Equals(b);
        }
        public static bool operator !=(Point a, Point b)
        {
            return !a.Equals(b);
        }
        #endregion
        #region Public methods
        public override bool Equals(object obj)
        {
            return (obj is Point) && Equals((Point)obj);
        }
        public bool Equals(Point other)
        {
            return ((X == other.X) && (Y == other.Y));
        }
        public override int GetHashCode()
        {
            unchecked
            {
                var hash = 17;
                hash = hash * 23 + X.GetHashCode();
                hash = hash * 23 + Y.GetHashCode();
                return hash;
            }
        }
        public override string ToString()
        {
            return "{X:" + X + " Y:" + Y + "}";
        }
        public Vector2 ToVector2()
        {
            return new Vector2(X, Y);
        }
        public void Deconstruct(out int x, out int y)
        {
            x = X;
            y = Y;
        }
        #endregion
    }
}