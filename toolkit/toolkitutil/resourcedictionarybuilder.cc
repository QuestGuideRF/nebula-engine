//------------------------------------------------------------------------------
//  resourcedictionarybuilder.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"

#include "core/refcounted.h"
#include "resources/streaming/resourceinfo.h"
#include "coregraphics/pixelformat.h"
#include "resourcedictionarybuilder.h"
#include "io/ioserver.h"
#include "io/stream.h"
#include "io/binarywriter.h"
#include "io/uri.h"
#include "util/stringatom.h"
#include "system/byteorder.h"
#include "resources/streaming/textureinfo.h"
#include "resources/streaming/poolresourcemapper.h"
#include "io/xmlwriter.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;
using namespace System;
using namespace Resources;

//------------------------------------------------------------------------------
/**
*/
ResourceDictionaryBuilder::ResourceDictionaryBuilder(void) :
    platform(Platform::Win32)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ResourceDictionaryBuilder::~ResourceDictionaryBuilder(void)
{
    this->dict.Clear();    
}

//------------------------------------------------------------------------------
/**
*/
bool
ResourceDictionaryBuilder::ReadTextureData(Ptr<Stream>& stream, TextureInfo& format)
{
    n_assert(stream->IsOpen());
    bool invalid = false;

    void* srcData = stream->Map();
    uint srcDataSize = stream->GetSize();

#if __DX11__
    HRESULT hr;
    D3DX11_IMAGE_INFO imageInfo = {0};
    hr = D3DX11GetImageInfoFromMemory(srcData, srcDataSize, NULL, &imageInfo, NULL);
#elif (__OGL4__ || __VULKAN__)
    ILint imageInfo = ilGenImage();
    ilBindImage(imageInfo);
    ILboolean success = ilLoadL(IL_DDS, srcData, srcDataSize);
#elif __DX9__
    HRESULT hr;
    D3DXIMAGE_INFO imageInfo = {0};
    hr = D3DXGetImageInfoFromFileInMemory(srcData, srcDataSize, &imageInfo);
#endif

#if __DX11__ || __DX9__
    if (FAILED(hr))
    {
        n_error("ResourceDictionaryBuilder: не удалось получить информацию об изображении из файла «%s»!", stream->GetURI().AsString());
    }
#elif __OGL4__
    if (!success)
    {
        n_error("ResourceDictionaryBuilder: не удалось получить информацию об изображении из файла «%s»!", stream->GetURI().AsString().AsCharPtr());
    }
#endif
    format.SetInfo(imageInfo);
    format.SetSize(stream->GetSize());
    // проверьте, действителен ли формат
    if (format.GetDepth() == 0 ||
        format.GetHeight() == 0 ||
        format.GetWidth() == 0 ||
        format.GetMipLevels() == 0 ||
        format.GetPixelFormat() == CoreGraphics::PixelFormat::InvalidPixelFormat ||
        format.GetType() == CoreGraphics::Texture::InvalidType ||
        format.GetSize() == 0)
    {
#ifdef DEBUG_RES_DICT_BUILDER
        n_error("ResourceDictionaryBuilder::ReadTextureData(): неверная текстура '%s':\n"
            "высота: %i\ширина: %i\n"
            "глубина: %i\tmipLevels: %i\n"
            "Error 500 (Server Error)!!1500.That’s an error.There was an error. Please try again later.That’s all we know."
            "размер: %i",
            stream->GetURI().AsString().AsCharPtr(), format.GetHeight(), format.GetWidth(), format.GetDepth(),
            format.GetMipLevels(), CoreGraphics::PixelFormat::ToString(format.GetPixelFormat()).AsCharPtr(), format.GetType(), format.GetSize());
#endif
        invalid = true;
    }

#if __OGL4__
    ilDeleteImage(imageInfo);
#endif
    stream->Unmap();
    
    return invalid;
}

//------------------------------------------------------------------------------
/**
*/
bool
ResourceDictionaryBuilder::BuildDictionary()
{
    n_assert(this->texDir.IsValid());
    n_assert(this->dict.IsEmpty());

    if (!this->AddTexturesToDictionary())
    {
        return false;
    }
    if (!this->SaveDictionary())
    {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ResourceDictionaryBuilder::AddTexturesToDictionary()
{
    // for each category...
    Array<String> categories = IoServer::Instance()->ListDirectories(URI(this->texDir), "*");
    IndexT catIndex;
    for (catIndex = 0; catIndex < categories.Size(); catIndex++)
    {
        // игнорировать каталоги системы контроля версий
        if ((categories[catIndex] != "CVS") && (categories[catIndex] != ".svn"))
        {
            // for each texture in category...
            String catDir;
            catDir.Format("%s/%s", this->texDir.AsCharPtr(), categories[catIndex].AsCharPtr());
            String pattern;
            pattern = "*.dds";
            Array<String> files = IoServer::Instance()->ListFiles(catDir, pattern);
            IndexT fileIndex;
            for (fileIndex = 0; fileIndex < files.Size(); fileIndex++)
            {
                // построить путь ABS и строку идентификатора ресурса
                String absPath;
                absPath.Format("%s/%s/%s", this->texDir.AsCharPtr(), categories[catIndex].AsCharPtr(), files[fileIndex].AsCharPtr());
                String resId;
                resId.Format("текс:%s/%s", categories[catIndex].AsCharPtr(), files[fileIndex].AsCharPtr());

                // получить размер файла ресурсов, нас не волнуют накладные расходы заголовка
                Ptr<Stream> stream = IoServer::Instance()->CreateStream(absPath);
                TextureInfo texInfo;
                stream->SetAccessMode(Stream::ReadAccess);
                if (false == stream->Open())
                {
                    n_printf("ОШИБКА, ПРОПУСКАЕМЫЙ РЕСУРС '%s', поскольку файл не может быть прочитан\n", absPath.AsCharPtr());
                    continue;
                }
                bool skip = this->ReadTextureData(stream, texInfo);
                stream->Close();
                stream = 0;
                if (true == skip)
                {
                    n_printf("ОШИБКА, ПРОПУСКАЕМЫЙ РЕСУРС '%s', поскольку один или несколько из следующих данных недействительны:\t"
                        "высота: %i\ширина: %i\t"
                        "глубина: %i\tmipLevels: %i\t"
                        "PixelFormat: '%s'\ttype: %i\t"
                        "размер: %i\n",
                        absPath.AsCharPtr(),
                        texInfo.GetHeight(), texInfo.GetWidth(),
                        texInfo.GetDepth(), texInfo.GetMipLevels(),
                        CoreGraphics::PixelFormat::ToString(texInfo.GetPixelFormat()).AsCharPtr(), texInfo.GetType(),
                        texInfo.GetSize());
                }
                else
                {
                    n_printf("Добавление '%s' в словарь.\t", resId.AsCharPtr());
#if DEBUG_RES_DICT_BUILDER
                    n_printf("высота: %i\ширина: %i\t"
                        "глубина: %i\tmipLevels: %i\t"
                        "PixelFormat: '%s'\ttype: %i\t"
                        "размер: %i\n",
                        texInfo.GetHeight(), texInfo.GetWidth(),
                        texInfo.GetDepth(), texInfo.GetMipLevels(),
                        CoreGraphics::PixelFormat::ToString(texInfo.GetPixelFormat()).AsCharPtr(), texInfo.GetType(),
                        texInfo.GetSize());
#endif
                    
                    this->dict.Add(resId, texInfo);
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ResourceDictionaryBuilder::SaveDictionary()
{
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(URI(this->texDir + "/resources.dic"));
    Ptr<BinaryWriter> writer = BinaryWriter::Create();
    writer->SetStream(stream);
    if (Platform::Win32 == this->platform)
    {
        writer->SetStreamByteOrder(ByteOrder::LittleEndian);
    }
    else
    {
        writer->SetStreamByteOrder(ByteOrder::BigEndian);
    }
    if (writer->Open())
    {
        // написать заголовок
        FourCC magic('RDIC');
        writer->WriteUInt(magic.AsUInt());      // магическое число «RDIC»
        writer->WriteUInt(2);                   // версия (0002)
        writer->WriteUInt(this->dict.Size());   // количество статей в словаре

        // for each dictionary entry...
        IndexT i;
        for (i = 0; i < this->dict.Size(); i++)
        {
            const StringAtom& resId = this->dict.KeyAtIndex(i);
            TextureInfo* texFormat = &this->dict.ValueAtIndex(i);
            n_assert(resId.AsString().Length() < MaxResIdLength);

            // скопировать содержимое идентификатора ресурса в буфер памяти
            uchar buf[MaxResIdLength] = { 0 };
            Memory::Copy(resId.Value(), buf, resId.AsString().Length());

            // Текстура::Type и PixelFormat являются перечислениями и поэтому записываются как int.
            // - если это может вызвать проблемы на платформах, измените тип данных на другой
            // здесь и в соответствующих классах Resource::ResourceDictionaryReader

            // записать текущую запись в файл
            writer->WriteInt(int(texFormat->GetType()));
            writer->WriteUInt(texFormat->GetWidth());
            writer->WriteUInt(texFormat->GetHeight());
            writer->WriteUInt(texFormat->GetDepth());
            writer->WriteUInt(texFormat->GetMipLevels());
            writer->WriteInt(int(texFormat->GetPixelFormat()));
            writer->WriteUInt(texFormat->GetSize());
            writer->WriteRawData(buf, sizeof(buf));
        }
        writer->Close();
        // тестовый пул находки с 500 МБ
        this->CreateDefaultTexturePool(URI(this->texDir + "/default_pool.xml"), __maxTextureBytes__);
        return true;
    }
    else
    {
        n_printf("ОШИБКА: не удалось открыть «%s» для записи!\n", this->texDir.AsCharPtr());
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceDictionaryBuilder::Unload()
{
    this->dict.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceDictionaryBuilder::CreateDefaultTexturePool(const URI& fileName, uint maxBytes)
{
    n_printf("создание пулов по умолчанию ...\n");
    // создайте хотя бы один пул с одним слотом на текстуру для пула по умолчанию
    Dictionary<StringAtom, PoolSetupInfo> slotsPerType;
    Dictionary<StringAtom, int> numSlotRequests;
    // зарезервировать некоторое начальное буферное пространство
    slotsPerType.Reserve(100);
    uint bytesLeft = maxBytes;
    IndexT dictIdx, addedIdx;
    StringAtom poolName;
    for (dictIdx = 0; dictIdx < this->dict.Size(); dictIdx++)
    {
        poolName.Clear();
        const TextureInfo* curInfo = &this->dict.ValueAtIndex(dictIdx);
        for (addedIdx = 0; addedIdx < slotsPerType.Size(); addedIdx++)
        {
            if (curInfo->IsEqual(slotsPerType.ValueAtIndex(addedIdx).info))
            {
                poolName = slotsPerType.KeyAtIndex(addedIdx);
                break;
            }
        }

        if (false == poolName.IsValid())
        {
            bytesLeft -= curInfo->GetSize();
            // убедитесь, что мы можем добавить новый пул с одним слотом
            n_assert2(bytesLeft >= 0, "Error: ResourceDictionaryBuiler::CreateDefaultTexturePool() - not enough space given to create at least one resource slot per Texture type. Size given: ");
            // Минимум 1 слот и тип слота по запросу...
            StringAtom poolId = CoreGraphics::PixelFormat::ToString(curInfo->GetPixelFormat()) + String::FromInt(curInfo->GetWidth()) + String::FromInt(curInfo->GetHeight()) + String::FromInt(curInfo->GetMipLevels());
            PoolSetupInfo newPool;
            newPool.numSlots = 1;
            newPool.info = (ResourceInfo*)curInfo;
            slotsPerType.Add(poolId, newPool);
            numSlotRequests.Add(poolId, 1);
        }
        else
        {
            // увеличить счетчик запрошенного слота
            numSlotRequests[poolName]++;
        }
    }

    n_printf("байт осталось после добавления 1 слота в пул:%uКБ из %u всего КБ\n", bytesLeft / 1024, maxBytes / 1024);

    // оценить словарь numSlots
    for (dictIdx = 0; dictIdx < numSlotRequests.Size(); dictIdx++)
    {
        n_assert(slotsPerType.Contains(numSlotRequests.KeyAtIndex(dictIdx)) && numSlotRequests.ValueAtIndex(dictIdx) > 0);
    }

    IndexT highestPoolRequestCounter, poolIdx;
    StringAtom highestPoolRequest;
    uint totalLoopCount = 0;
    bool allSlotsEnabled = false;
    while (bytesLeft > 0)
    {
        highestPoolRequestCounter = -1;
        highestPoolRequest.Clear();
        for (poolIdx = 0; poolIdx < numSlotRequests.Size(); poolIdx++)
        {
            // if slot has more requests than any previous and there is enough space left, set this slot as highest priority for slot-increase
            if (numSlotRequests.ValueAtIndex(poolIdx) > highestPoolRequestCounter &&
                slotsPerType[numSlotRequests.KeyAtIndex(poolIdx)].numSlots < numSlotRequests.ValueAtIndex(poolIdx) &&
                slotsPerType[numSlotRequests.KeyAtIndex(poolIdx)].info->GetSize() <= bytesLeft)
            {
                highestPoolRequestCounter = numSlotRequests.ValueAtIndex(poolIdx);
                highestPoolRequest = numSlotRequests.KeyAtIndex(poolIdx);
            }
        }
        totalLoopCount++;
        if (highestPoolRequest.IsValid())
        {
            slotsPerType[highestPoolRequest].numSlots++;
            bytesLeft -= slotsPerType[highestPoolRequest].info->GetSize();
        }
        else
        {
            allSlotsEnabled = true;
            break;
        }
    }
    n_printf("создание пула текстур по умолчанию: количество циклов(%u), количество циклов *pools(%u)\n", totalLoopCount, totalLoopCount * (uint)slotsPerType.Size());
    n_printf("байт осталось после создания максимального количества слотов в пуле:%uКБ из %u всего КБ\n", bytesLeft / 1024, maxBytes / 1024);
    if (allSlotsEnabled)
    {
        n_printf("достаточно места для загрузки всех текстур!\n");
    }
    PoolResourceMapper::WriteTexturePoolToXML(fileName, slotsPerType);
}
} // пространство имен ToolkitUtil
