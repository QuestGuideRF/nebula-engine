#pragma once
#include "math/float4.h"
#include "math/quaternion.h"
#include "coreanimation/animcurve.h"
#include "characters/characterjointmask.h"
namespace CoreAnimation
{
using namespace Math;
inline void
AnimJobUtilSampleStep(const AnimCurve* curves,
                      int numCurves,
                      const float4& velocityScale,
                      const float4* src0SamplePtr,
                      float4* outSamplePtr,
                      uchar* outSampleCounts)
{
    float4 f0;
    int i;
    for (i = 0; i < numCurves; i++)
    {
        const AnimCurve& curve = curves[i];
        if (!curve.IsActive())
        {
            outSampleCounts[i] = 0;
        }
        else
        {
            outSampleCounts[i] = 1;
            if (curve.IsStatic())
            {
                f0 = curve.GetStaticKey();
            }
            else
            {
                f0.load((scalar*)src0SamplePtr);
                src0SamplePtr++;
            }
            if (curve.GetCurveType() == CurveType::Velocity)
            {
                f0 = float4::multiply(f0, velocityScale);
            }
            f0.store((scalar*)outSamplePtr);
        }
        outSamplePtr++;
    }
}
inline void
AnimJobUtilSampleLinear(const AnimCurve* curves,
                        int numCurves,
                        float sampleWeight,
                        const float4& velocityScale,
                        const float4* src0SamplePtr,
                        const float4* src1SamplePtr,
                        float4* outSamplePtr,
                        uchar* outSampleCounts)
{
    float4 f0, f1, fDst;
    quaternion q0, q1, qDst;
    int i;
    for (i = 0; i < numCurves; i++)
    {
        const AnimCurve& curve = curves[i];
        if (!curve.IsActive())
        {
            outSampleCounts[i] = 0;
        }
        else
        {
            CurveType::Code curveType = curve.GetCurveType();
            outSampleCounts[i] = 1;
            if (curve.IsStatic())
            {
                f0 = curve.GetStaticKey();
                if (CurveType::Velocity == curveType)
                {
                    f0 = float4::multiply(f0, velocityScale);
                }
                f0.store((scalar*)outSamplePtr);
            }
            else
            {
                if (curve.GetCurveType() == CurveType::Rotation)
                {
                    q0.load((scalar*)src0SamplePtr);
                    q1.load((scalar*)src1SamplePtr);
                    qDst = quaternion::slerp(q0, q1, sampleWeight);
                    qDst.store((scalar*)outSamplePtr);
                }
                else
                {
                    f0.load((scalar*)src0SamplePtr);
                    f1.load((scalar*)src1SamplePtr);
                    fDst = float4::lerp(f0, f1, sampleWeight);
                    if (CurveType::Velocity == curveType)
                    {
                        fDst = float4::multiply(fDst, velocityScale);
                    }
                    fDst.store((scalar*)outSamplePtr);
                }
                src0SamplePtr++;
                src1SamplePtr++;
            }
        }
        outSamplePtr++;
    }
}
inline void
AnimJobUtilMix(const AnimCurve* curves,
               int numCurves,
               const Characters::CharacterJointMask* mask,
               float mixWeight,
               const float4* src0SamplePtr,
               const float4* src1SamplePtr,
               const uchar* src0SampleCounts,
               const uchar* src1SampleCounts,
               float4* outSamplePtr,
               uchar* outSampleCounts)
{
    float4 f0, f1, fDst;
    quaternion q0, q1, qDst;
    int i;
    for (i = 0; i < numCurves; i++)
    {
        const AnimCurve& curve = curves[i];
        uchar src0Count = src0SampleCounts[i];
        uchar src1Count = src1SampleCounts[i];
        outSampleCounts[i] = src0Count + src1Count;
        if ((src0Count > 0) && (src1Count > 0))
        {
            float maskWeight = 1;
            if (mask != 0) maskWeight = mask->GetWeight(i/4);
            if (curve.GetCurveType() == CurveType::Rotation)
            {
                q0.load((scalar*)src0SamplePtr);
                q1.load((scalar*)src1SamplePtr);
                qDst = quaternion::slerp(q0, q1, mixWeight * maskWeight);
                qDst.store((scalar*)outSamplePtr);
            }
            else
            {
                f0.load((scalar*)src0SamplePtr);
                f1.load((scalar*)src1SamplePtr);
                fDst = float4::lerp(f0, f1, mixWeight * maskWeight);
                fDst.store((scalar*)outSamplePtr);
            }
        }
        else if (src0Count > 0)
        {
            f0.load((scalar*)src0SamplePtr);
            f0.store((scalar*)outSamplePtr);
        }
        else if (src1Count > 0)
        {
            f1.load((scalar*)src1SamplePtr);
            f1.store((scalar*)outSamplePtr);
        }
        else
        {
        }
        src0SamplePtr++;
        src1SamplePtr++;
        outSamplePtr++;
    }
}
}