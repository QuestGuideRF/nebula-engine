//------------------------------------------------------------------------------
//  bruteforcesystem.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "bruteforcesystem.h"
#include "jobs2/jobs2.h"
#include "math/mat4.h"
#include "math/clipstatus.h"
namespace Visibility
{

//------------------------------------------------------------------------------
/**
*/
void
BruteforceSystem::Setup(const BruteforceSystemLoadInfo& info)
{
}

//------------------------------------------------------------------------------
/**
*/
void
BruteforceSystem::Run(const Threading::AtomicCounter* previousSystemCompletionCounters, const Util::FixedArray<const Threading::AtomicCounter*, true>& extraCounters)
{
    IndexT i;
    for (i = 0; i < this->obs.count; i++)
    {
        Math::mat4 camera = this->obs.transforms[i];

        n_assert(this->obs.completionCounters[i] == 0);
        this->obs.completionCounters[i] = 1;

        // Настройка счетчиков
        Util::FixedArray<const Threading::AtomicCounter*, true> counters(extraCounters.Size() + (previousSystemCompletionCounters == nullptr ? 0 : 1));
        if (!extraCounters.IsEmpty())
            Memory::CopyElements(extraCounters.Begin(), counters.Begin(), extraCounters.Size());
        if (previousSystemCompletionCounters != nullptr)
            counters[extraCounters.Size()] = &previousSystemCompletionCounters[i];

        // Распределите матрицу так, чтобы все _x, _y,... содержали значения столбцов x, y,...
        // Это дает возможность перестроить преобразование камеры в более дружественное к SSE матричное преобразование в задании.
        Math::vec4 colX[4], colY[4], colZ[4], colW[4];
        colX[0] = Math::splat_x((camera).r[0]);
        colX[1] = Math::splat_x((camera).r[1]);
        colX[2] = Math::splat_x((camera).r[2]);
        colX[3] = Math::splat_x((camera).r[3]);

        colY[0] = Math::splat_y((camera).r[0]);
        colY[1] = Math::splat_y((camera).r[1]);
        colY[2] = Math::splat_y((camera).r[2]);
        colY[3] = Math::splat_y((camera).r[3]);

        colZ[0] = Math::splat_z((camera).r[0]);
        colZ[1] = Math::splat_z((camera).r[1]);
        colZ[2] = Math::splat_z((camera).r[2]);
        colZ[3] = Math::splat_z((camera).r[3]);

        colW[0] = Math::splat_w((camera).r[0]);
        colW[1] = Math::splat_w((camera).r[1]);
        colW[2] = Math::splat_w((camera).r[2]);
        colW[3] = Math::splat_w((camera).r[3]);

        // Все готово, запускаем задание
        Jobs2::JobDispatch(
            [
                ids = this->ent.ids
                , boundingBoxes = this->ent.boxes
                , flags = this->ent.entityFlags
                , isOrtho = this->obs.isOrtho[i]
                , clipStatuses = this->obs.results[i].Begin()
                , colX, colY, colZ, colW
            ]
        (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
        {
            N_SCOPE(BruteforceViewFrustumCulling, Visibility);
            // Перебрать рабочую группу
            for (IndexT i = 0; i < groupSize; i++)
            {
                // Получить индекс элемента
                IndexT index = i + invocationOffset;
                if (index >= totalJobs)
                    return;

                uint32_t objectId = ids[index];

                if (AllBits(flags[objectId], (uint32_t)Models::NodeInstanceFlags::NodeInstance_AlwaysVisible))
                {
                    clipStatuses[index] = Math::ClipStatus::Inside;
                    continue;
                }

                // Запустите проверку ограничивающего прямоугольника и сохраните выходные данные в статусах клипа, если статус клипа все еще находится снаружи.
                if (clipStatuses[index] == Math::ClipStatus::Outside)
                    clipStatuses[index] = boundingBoxes[objectId].clipstatus(colX, colY, colZ, colW, isOrtho);
            }
        }
        , this->ent.count
        , 1024
        , counters
        , { &this->obs.completionCounters[i] }
        , nullptr);
    }
}
} // Видимость пространства имен
