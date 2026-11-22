using System;
namespace Mathf
{
    public static class MathHelper
    {
        public const float E = (float)System.Math.E;
        public const float Log10E = 0.4342945f;
        public const float Log2E = 1.442695f;
        public const float Pi = (float)System.Math.PI;
        public const float PiOver2 = (float)(System.Math.PI / 2.0);
        public const float PiOver4 = (float)(System.Math.PI / 4.0);
        public const float TwoPi = (float)(System.Math.PI * 2.0);
        public static float Barycentric(float value1, float value2, float value3, float amount1, float amount2)
        {
            return value1 + (value2 - value1) * amount1 + (value3 - value1) * amount2;
        }
        public static float CatmullRom(float value1, float value2, float value3, float value4, float amount)
        {
            double amountSquared = amount * amount;
            double amountCubed = amountSquared * amount;
            return (float)(0.5 * (2.0 * value2 +
                (value3 - value1) * amount +
                (2.0 * value1 - 5.0 * value2 + 4.0 * value3 - value4) * amountSquared +
                (3.0 * value2 - value1 - 3.0 * value3 + value4) * amountCubed));
        }
        public static float Clamp(float value, float min, float max)
        {
            value = (value > max) ? max : value;
            value = (value < min) ? min : value;
            return value;
        }
        public static int Clamp(int value, int min, int max)
        {
            value = (value > max) ? max : value;
            value = (value < min) ? min : value;
            return value;
        }
        public static float Distance(float value1, float value2)
        {
            return System.Math.Abs(value1 - value2);
        }
        public static float Hermite(float value1, float tangent1, float value2, float tangent2, float amount)
        {
            double v1 = value1, v2 = value2, t1 = tangent1, t2 = tangent2, s = amount, result;
            double sCubed = s * s * s;
            double sSquared = s * s;
            if (amount == 0f)
                result = value1;
            else if (amount == 1f)
                result = value2;
            else
                result = (2 * v1 - 2 * v2 + t2 + t1) * sCubed +
                    (3 * v2 - 3 * v1 - 2 * t1 - t2) * sSquared +
                    t1 * s +
                    v1;
            return (float)result;
        }
        public static float Lerp(float value1, float value2, float amount)
        {
            return value1 + (value2 - value1) * amount;
        }
        public static float LerpPrecise(float value1, float value2, float amount)
        {
            return ((1 - amount) * value1) + (value2 * amount);
        }
        public static float Max(float value1, float value2)
        {
            return value1 > value2 ? value1 : value2;
        }
        public static int Max(int value1, int value2)
        {
            return value1 > value2 ? value1 : value2;
        }
        public static float Min(float value1, float value2)
        {
            return value1 < value2 ? value1 : value2;
        }
        public static int Min(int value1, int value2)
        {
            return value1 < value2 ? value1 : value2;
        }
        public static float SmoothStep(float value1, float value2, float amount)
        {
            float result = MathHelper.Clamp(amount, 0f, 1f);
            result = MathHelper.Hermite(value1, 0f, value2, 0f, result);
            return result;
        }
        public static float ToDegrees(float radians)
        {
            return (float)(radians * 57.295779513082320876798154814105);
        }
        public static float ToRadians(float degrees)
        {
            return (float)(degrees * 0.017453292519943295769236907684886);
        }
        public static float WrapAngle(float angle)
        {
            if ((angle > -Pi) && (angle <= Pi))
                return angle;
            angle %= TwoPi;
            if (angle <= -Pi)
                return angle + TwoPi;
            if (angle > Pi)
                return angle - TwoPi;
            return angle;
        }
	public static bool IsPowerOfTwo(int value)
	{
	     return (value > 0) && ((value & (value - 1)) == 0);
	}
    }
}