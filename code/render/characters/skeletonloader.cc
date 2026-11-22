//------------------------------------------------------------------------------
//  skeletonloader.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "skeletonloader.h"
#include "nskfileformatstructs.h"
#include "util/fourcc.h"
#include "skeletonresource.h"
using namespace IO;
namespace Characters
{

__ImplementClass(Characters::SkeletonLoader, 'SSKP', Resources::ResourceLoader)

//------------------------------------------------------------------------------
/**
*/
Resources::ResourceLoader::ResourceInitOutput
SkeletonLoader::InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream)
{
    // буфер карты
    Resources::ResourceLoader::ResourceInitOutput ret;
    byte* ptr = (byte*)stream->Map();

    // прочитать заголовок
    Nsk3Header* header = (Nsk3Header*)ptr;
    ptr += sizeof(Nsk3Header);

    // проверить магическое значение
    if (Util::FourCC(header->magic) != NEBULA_NSK3_MAGICNUMBER)
    {
        n_error("StreamSkeletonCache::InitializeResource(): '%s' имеет неверный формат файла (магический номер не совпадает)!", stream->GetURI().AsString().AsCharPtr());
        return ret;
    }

    Util::FixedArray<SkeletonId> skeletons(header->numSkeletons);
    skeletons.Fill(InvalidSkeletonId);
    for (uint skeletonIndex = 0; skeletonIndex < header->numSkeletons; skeletonIndex++)
    {
        Nsk3Skeleton* nsk3Skeleton = (Nsk3Skeleton*)ptr;
        ptr += sizeof(Nsk3Skeleton);

        // нагрузочные соединения
        if (nsk3Skeleton->numJoints > 0)
        {
            Util::FixedArray<CharacterJoint> joints;
            Util::FixedArray<Math::mat4> bindPoses;
            Util::HashTable<Util::StringAtom, IndexT> jointIndexMap;
            Util::FixedArray<Math::vec4> idleSamples;

            joints.SetSize(nsk3Skeleton->numJoints);
            bindPoses.SetSize(nsk3Skeleton->numJoints);
            idleSamples.SetSize(nsk3Skeleton->numJoints * 3);
            uint jointIndex;
            for (jointIndex = 0; jointIndex < nsk3Skeleton->numJoints; jointIndex++)
            {
                Nsk3Joint* joint = (Nsk3Joint*)ptr;
                ptr += sizeof(Nsk3Joint);

                // настройка базовых компонентов
                joints[jointIndex].parentJointIndex = joint->parent;
                if (joint->parent != InvalidIndex)
                    joints[jointIndex].parentJoint = &joints[joint->parent];
                else
                    joints[jointIndex].parentJoint = nullptr;

#if NEBULA_DEBUG
                joints[jointIndex].name = joint->name;
#endif

                // настройка позы привязки и картографирования
                bindPoses[jointIndex].loadu(joint->bind);
                jointIndexMap.Add(joint->name, jointIndex);

                idleSamples[jointIndex * 3 + 0].loadu(joint->translation);
                idleSamples[jointIndex * 3 + 0].w = 0.0f;
                idleSamples[jointIndex * 3 + 1].loadu(joint->rotation);
                idleSamples[jointIndex * 3 + 2].loadu(joint->scale);
                idleSamples[jointIndex * 3 + 2].w = 0.0f;
            }

            SkeletonCreateInfo info;
            info.joints = joints;
            info.bindPoses = bindPoses;
            info.jointIndexMap = jointIndexMap;
            info.idleSamples = idleSamples;
            SkeletonId skeleton = CreateSkeleton(info);
            skeletons[skeletonIndex] = skeleton;
        }
    }
    stream->Unmap();

    auto id = skeletonResourceAllocator.Alloc();
    skeletonResourceAllocator.Set<0>(id, skeletons);
    ret.id = id;
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void 
SkeletonLoader::Unload(const Resources::ResourceId id)
{
    DestroySkeletonResource(id);
}

} // Символы пространства имен
