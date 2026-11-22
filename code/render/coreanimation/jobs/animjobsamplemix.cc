//------------------------------------------------------------------------------
//  animjobsamplemix.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "jobs/stdjob.h"
#include "animjobutil.h"
#include "coreanimation/animsamplemixinfo.h"
#include "characters/characterjointmask.h"

namespace CoreAnimation
{

//------------------------------------------------------------------------------
/**
    Выполняет как выборку анимации, так и смешивание с другим образцом.
    буфер в одном задании.
*/
void
AnimSampleMixJobFunc(const JobFuncContext& ctx)
{
    const AnimCurve* animCurves = (const AnimCurve*) ctx.uniforms[0];
    int numCurves = ctx.uniformSizes[0] / sizeof(AnimCurve);
    const AnimSampleMixInfo* info = (const AnimSampleMixInfo*) ctx.uniforms[1];
    const Characters::CharacterJointMask* mask = (const Characters::CharacterJointMask*)ctx.uniforms[2];
    const float4* src0SamplePtr = (const float4*) ctx.inputs[0];
    const float4* src1SamplePtr = (const float4*) ctx.inputs[1];
    const float4* mixSamplePtr  = (const float4*) ctx.inputs[2];
    float4* tmpSamplePtr   = (float4*) ctx.scratch;
    uchar* tmpSampleCounts = (uchar*) (tmpSamplePtr + numCurves);
    uchar* mixSampleCounts = ctx.inputs[3];
    float4* outSamplePtr = (float4*) ctx.outputs[0];
    uchar* outSampleCounts = ctx.outputs[1];

    // сначала выполните этап выборки...
    if (SampleType::Step == info->sampleType)
    {
        AnimJobUtilSampleStep(animCurves, numCurves, info->velocityScale, src0SamplePtr, tmpSamplePtr, tmpSampleCounts);
    }
    else
    {
        AnimJobUtilSampleLinear(animCurves, numCurves, info->sampleWeight, info->velocityScale, src0SamplePtr, src1SamplePtr, tmpSamplePtr, tmpSampleCounts);
    }
    
    // ...затем смешиваем, ПРИМЕЧАНИЕ: outSamplePtr и outSampleCounts считываются и перезаписываются!
    AnimJobUtilMix(animCurves, numCurves, mask, info->mixWeight, mixSamplePtr, tmpSamplePtr, mixSampleCounts, tmpSampleCounts, outSamplePtr, outSampleCounts);
}

} // пространство имен CoreAnimation
__ImplementSpursJob(CoreAnimation::AnimSampleMixJobFunc);
