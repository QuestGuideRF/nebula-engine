//------------------------------------------------------------------------------
//  skeletonevaljob.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "jobs/jobs.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "characters/skeletonjoint.h"
#include "profiling/profiling.h"

using namespace Math;
namespace Characters
{

//------------------------------------------------------------------------------
/**
*/
void
SkeletonEvalJob(const Jobs::JobFuncContext& ctx)
{
    N_SCOPE_ACCUM(SkeletonEvaluateAndTransform, Character);
    mat4* invPoseMatrixBase = (mat4*)ctx.uniforms[0];
    mat4* mixPoseMatrixBase = (mat4*)ctx.uniforms[1];
    mat4* unscaledMatrixBase = (mat4*)ctx.scratch;

    for (ptrdiff sliceIdx = 0; sliceIdx < ctx.numSlices; sliceIdx++)
    {
        vec4 finalScale, parentFinalScale, variationTranslation;
        vec4 translate(0.0f, 0.0f, 0.0f, 1.0f);
        vec4 scale(1.0f, 1.0f, 1.0f, 0.0f);
        vec4 parentScale(1.0f, 1.0f, 1.0f, 0.0f);
        vec4 parentTranslate(0.0f, 0.0f, 0.0f, 1.0f);
        quat rotate;
        vec4 vec1111(1.0f, 1.0f, 1.0f, 1.0f);

        // загрузить указатели из контекста
        // ПРИМЕЧАНИЕ. Указатель sampleBase может иметь значение NULL, если нет допустимой анимации.
        // данные существуют, в этом случае скелет должен быть просто установлен
        // в позу Иисуса
        SkeletonJobJoint* compsBase = (SkeletonJobJoint*)(ctx.inputs[0] + sliceIdx * ctx.inputSizes[0]);
        n_assert(0 != compsBase);
        const vec4* samplesBase = (const vec4*)(ctx.inputs[1] + sliceIdx * ctx.inputSizes[1]);
        const vec4* samplesPtr = samplesBase;

        mat4* scaledMatrixBase = (mat4*)(ctx.outputs[0] + sliceIdx * ctx.outputSizes[0]);
        mat4* skinMatrixBase = (mat4*)(ctx.outputs[1] + sliceIdx * ctx.outputSizes[1]);

        // входные сэмплы могут дополнительно включать сэмплы скорости, которые нам нужно пропустить...
        uint numElements = ctx.inputSizes[0] / sizeof(SkeletonJobJoint);
        uint sampleWidth = (ctx.inputSizes[1] / numElements) / sizeof(vec4);

        // вычислить количество суставов
        int jointIndex;
        int numJoints = ctx.inputSizes[0] / sizeof(SkeletonJobJoint);
        for (jointIndex = 0; jointIndex < numJoints; jointIndex++)
        {
            // Нагрузочное соединение перемещается/поворачивается/масштабируется
            translate.load((scalar*)&(samplesPtr[0]));
            rotate.load((scalar*)&(samplesPtr[1]));
            scale.load((scalar*)&(samplesPtr[2]));
            samplesPtr += sampleWidth;

            const SkeletonJobJoint& comps = compsBase[jointIndex];

            // шкала изменения нагрузки
            finalScale.load(&comps.varScaleX);
            finalScale = scale * finalScale;
            finalScale = permute(finalScale, vec1111, 0, 1, 2, 7);

            mat4& unscaledMatrix = unscaledMatrixBase[jointIndex];
            mat4& scaledMatrix = scaledMatrixBase[jointIndex];

            // обновить немасштабированную матрицу
            // вращение анимации
            unscaledMatrix = rotationquat(rotate);

            // перевод изменения нагрузки
            variationTranslation.load(&comps.varTranslationX);
            unscaledMatrix.translate(xyz(translate + variationTranslation));

            // добавить смешанную позу, если указатель установлен
            if (mixPoseMatrixBase)
                unscaledMatrix = mixPoseMatrixBase[jointIndex] * unscaledMatrix;

            // обновить масштабированную матрицу
            // масштаб после вращения
            scaledMatrix.x_axis = unscaledMatrix.x_axis * splat_x(finalScale);
            scaledMatrix.y_axis = unscaledMatrix.y_axis * splat_y(finalScale);
            scaledMatrix.z_axis = unscaledMatrix.z_axis * splat_z(finalScale);

            if (InvalidIndex == comps.parentJointIndex)
            {
                // ни один родитель не установил перевод напрямую
                scaledMatrix.position = unscaledMatrix.position;
            }
            else
            {
                // загрузить масштаб родительской анимации
                parentScale.load((scalar*)(samplesBase + sampleWidth * jointIndex + 2));
                const SkeletonJobJoint& parentComps = compsBase[comps.parentJointIndex];

                // объединить оба типа масштабирования
                parentFinalScale.load(&parentComps.varScaleX);
                parentFinalScale = parentScale * parentFinalScale;
                parentFinalScale = permute(parentFinalScale, vec1111, 0, 1, 2, 7);

                // трансформируйте нашу немасштабированную позицию с помощью родительского масштабирования
                unscaledMatrix.position = unscaledMatrix.position * parentFinalScale;
                scaledMatrix.position = unscaledMatrix.position;

                // применить вращение и относительный перевод анимации родительского элемента 
                const mat4& parentUnscaledMatrix = unscaledMatrixBase[comps.parentJointIndex];
                unscaledMatrix = parentUnscaledMatrix * unscaledMatrix;
                scaledMatrix = parentUnscaledMatrix * scaledMatrix;
            }
            skinMatrixBase[jointIndex] = scaledMatrix * invPoseMatrixBase[jointIndex];
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
SkeletonEvalJobWithVariation(const Jobs::JobFuncContext& ctx)
{
    N_SCOPE_ACCUM(SkeletonEvaluateAndTransformWithVariation, Character);
    mat4* invPoseMatrixBase = (mat4*)ctx.uniforms[0];
    mat4* mixPoseMatrixBase = (mat4*)ctx.uniforms[1];
    mat4* unscaledMatrixBase = (mat4*)ctx.scratch;

    for (ptrdiff sliceIdx = 0; sliceIdx < ctx.numSlices; sliceIdx++)
    {
        vec4 finalScale, parentFinalScale, variationTranslation;
        vec4 translate(0.0f, 0.0f, 0.0f, 1.0f);
        vec4 scale(1.0f, 1.0f, 1.0f, 0.0f);
        vec4 parentScale(1.0f, 1.0f, 1.0f, 0.0f);
        vec4 parentTranslate(0.0f, 0.0f, 0.0f, 1.0f);
        quat rotate;
        vec4 vec1111(1.0f, 1.0f, 1.0f, 1.0f);

        // загрузить указатели из контекста
        // ПРИМЕЧАНИЕ. Указатель sampleBase может иметь значение NULL, если нет допустимой анимации.
        // данные существуют, в этом случае скелет должен быть просто установлен
        // в позу Иисуса
        SkeletonJobJoint* compsBase = (SkeletonJobJoint*)N_JOB_INPUT(ctx, sliceIdx, 0);
        n_assert(0 != compsBase);
        const vec4* samplesBase = (const vec4*)N_JOB_INPUT(ctx, sliceIdx, 1);
        const vec4* samplesPtr = samplesBase;

        mat4* scaledMatrixBase = (mat4*)N_JOB_OUTPUT(ctx, sliceIdx, 0);
        mat4* skinMatrixBase = (mat4*)N_JOB_OUTPUT(ctx, sliceIdx, 1);

        // входные сэмплы могут дополнительно включать сэмплы скорости, которые нам нужно пропустить...
        uint numElements = ctx.inputSizes[0] / sizeof(SkeletonJobJoint);
        uint sampleWidth = (ctx.inputSizes[1] / numElements) / sizeof(vec4);

        // вычислить количество суставов
        int jointIndex;
        int numJoints = ctx.inputSizes[0] / sizeof(SkeletonJobJoint);
        for (jointIndex = 0; jointIndex < numJoints; jointIndex++)
        {
            // Нагрузочное соединение перемещается/поворачивается/масштабируется
            translate.load((scalar*)&(samplesPtr[0]));
            rotate.load((scalar*)&(samplesPtr[1]));
            scale.load((scalar*)&(samplesPtr[2]));
            samplesPtr += sampleWidth;

            const SkeletonJobJoint& comps = compsBase[jointIndex];

            // шкала изменения нагрузки
            finalScale.load(&comps.varScaleX);
            finalScale = scale * finalScale;
            finalScale = permute(finalScale, vec1111, 0, 1, 2, 7);

            mat4& unscaledMatrix = unscaledMatrixBase[jointIndex];
            mat4& scaledMatrix = scaledMatrixBase[jointIndex];

            // обновить немасштабированную матрицу
            // вращение анимации
            unscaledMatrix = rotationquat(rotate);

            // перевод изменения нагрузки
            variationTranslation.load(&comps.varTranslationX);
            unscaledMatrix.translate(xyz(translate + variationTranslation));

            // добавить смешанную позу, если указатель установлен
            if (mixPoseMatrixBase)  
                unscaledMatrix = mixPoseMatrixBase[jointIndex] * unscaledMatrix;

            // обновить масштабированную матрицу
            // масштаб после вращения
            scaledMatrix.x_axis = unscaledMatrix.x_axis * splat_x(finalScale);
            scaledMatrix.y_axis = unscaledMatrix.y_axis * splat_y(finalScale);
            scaledMatrix.z_axis = unscaledMatrix.z_axis * splat_z(finalScale);

            if (InvalidIndex == comps.parentJointIndex)
            {
                // ни один родитель не установил перевод напрямую
                scaledMatrix.position = unscaledMatrix.position;
            }
            else
            {
                // загрузить масштаб родительской анимации
                parentScale.load((scalar*)(samplesBase + sampleWidth * jointIndex + 2));
                const SkeletonJobJoint& parentComps = compsBase[comps.parentJointIndex];

                // загрузить шкалу родительских вариантов
                parentFinalScale.load(&parentComps.varScaleX);

                // объединить оба типа масштабирования
                parentFinalScale = parentScale * parentFinalScale;
                parentFinalScale = permute(parentFinalScale, vec1111, 0, 1, 2, 7);

                // трансформируйте нашу немасштабированную позицию с помощью родительского масштабирования
                unscaledMatrix.position = unscaledMatrix.position * parentFinalScale;
                scaledMatrix.position = unscaledMatrix.position;

                // применить вращение и относительный перевод анимации родительского элемента 
                const mat4& parentUnscaledMatrix = unscaledMatrixBase[comps.parentJointIndex];
                unscaledMatrix = parentUnscaledMatrix * unscaledMatrix;
                scaledMatrix = parentUnscaledMatrix * scaledMatrix;
            }
            skinMatrixBase[jointIndex] = scaledMatrix * invPoseMatrixBase[jointIndex];
        }
    }
}

} // Символы пространства имен
