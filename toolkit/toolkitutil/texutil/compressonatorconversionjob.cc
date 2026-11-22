//------------------------------------------------------------------------------
//  compressonatorconversionjob.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"

#include "compressonatorconversionjob.h"
#include "io/uri.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "timing/timer.h"

#include "Compressonator.h"

extern void (*PrintStatusLine)(char*);

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

void Printl(char* buf)
{
    fprintf(stderr,"%s\n", buf);
}

//------------------------------------------------------------------------------
/**
*/
CompressonatorConversionJob::CompressonatorConversionJob()
{
    this->SetDstFileExtension("dds");
    auto printline = [=](char*buff)->void { this->logger->Error(buff); };
    PrintStatusLine = &Printl;
}

//------------------------------------------------------------------------------
/**
    Запустите процесс конвертации. Возвращает false, если задание завершилось немедленно.   
*/
bool
CompressonatorConversionJob::Convert()
{
    n_assert(0 != this->logger);
    if (TextureConversionJob::Convert())
    {
        URI srcPathUri(this->srcPath);
        URI dstPathUri(this->dstPath);
        URI tmpDirUri(this->tmpDir);

        String src = srcPathUri.LocalPath();

        Timing::Timer timer;
        timer.Start();

        this->logger->Print("Обработка: %s\n", src.AsCharPtr());

        CMP_MipSet MipSetIn;
        Memory::Clear(&MipSetIn, sizeof(CMP_MipSet));

        auto cmp_status = CMP_LoadTexture(src.AsCharPtr(), &MipSetIn);

        if (cmp_status != CMP_OK)
        {
            this->logger->Error("Не удалось загрузить %s, код ошибки: %d\n", src.AsCharPtr(), cmp_status);
            return false;
        }


        if (MipSetIn.m_format != CMP_FORMAT_RGBA_8888)
        {
            this->logger->Print("Текстура без RGB(a), сохранение в формате Raw\n");

            cmp_status = CMP_SaveTexture(dstPathUri.LocalPath().AsCharPtr(), &MipSetIn);
            if (cmp_status != CMP_OK)
            {
                this->logger->Error("Не удалось загрузить %s, CMP_SaveTexture, код ошибки: %d\n", src.AsCharPtr(), cmp_status);
                return false;
            }

            CMP_FreeMipSet(&MipSetIn);

            Timing::Time after = timer.GetTime();
            this->logger->Print("Выполнено после: %f\n", after);
            return true;
        }


        if (MipSetIn.m_nMipLevels <= 1)
        {
            CMP_INT requestLevel = 10; // Запросить 10 миуровней для исходного изображения

            //------------------------------------------------------------------------
            // Проверяет, каким будет минимальный размер изображения для запрошенных уровней MIP.
            // if the request is too large, a adjusted minimum size will be returned
            //------------------------------------------------------------------------
            CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, 10);

            //--------------------------------------------------------------
            // теперь, когда известен минимальный размер, сгенерируйте miplevels
            // пользователи могут установить любой запрошенный минимальный размер для использования. Правильный
            // miplevels будут установлены соответствующим образом.
            //--------------------------------------------------------------
            CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
        }

        //==========================
        // Установить параметры сжатия
        //==========================
        KernelOptions   kernel_options;
        memset(&kernel_options, 0, sizeof(KernelOptions));

        const TextureAttrs& attrs = this->textureAttrs;
        float quality = 0.5f;
        switch (attrs.GetQuality())
        {
        case ToolkitUtil::TextureAttrs::High:
            quality = 0.7f;
            break;
        case ToolkitUtil::TextureAttrs::Low:
            quality = 0.1f;
            break;
        }
        if((String::MatchPattern(this->srcPath, "*норма.*")) 
            || (String::MatchPattern(this->srcPath, "*нормальный.*")) 
            || (String::MatchPattern(this->srcPath, "*Нормальный.*")) 
            || (String::MatchPattern(this->srcPath, "*ударяться.*")))
        {
            kernel_options.format = CMP_FORMAT_BC5;
        }
        else if (String::MatchPattern(this->srcPath, "*высота.*"))
        {
            kernel_options.format = CMP_FORMAT_R_16;
        }
        else
        {
            kernel_options.format = CMP_FORMAT_BC7; // Установите формат для обработки
        }
        kernel_options.fquality = quality;     // Установите качество результата
        kernel_options.threads = 0;             // Автоматическая настройка
        kernel_options.encodeWith = CMP_CPU;

        //--------------------------------------------------------------
        // Настройте буфер результатов для обработанного файла,
        // содержимое будет установлено после обработки исходной текстуры
        // в вызове CMP_ProcessTexture()
        //--------------------------------------------------------------
        CMP_MipSet MipSetCmp;
        memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

        CMP_Feedback_Proc CompressionCallback = [](CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2)->bool {
            return 0;
        };

        //===============================================
        // Сжать текстуру с помощью Framework Lib
        //===============================================
        cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, CompressionCallback);
        if (cmp_status != CMP_OK)
        {
            this->logger->Error("Не удалось загрузить %s, CMP_ProcessTexture, код ошибки: %d\n", src.AsCharPtr(), cmp_status);
            return false;
        }

        //----------------------------------------------------------------
        // Сохраните результат в файл DDS.
        //----------------------------------------------------------------
        cmp_status = CMP_SaveTexture(dstPathUri.LocalPath().AsCharPtr(), &MipSetCmp);
        if (cmp_status != CMP_OK)
        {
            this->logger->Error("Не удалось загрузить %s, CMP_SaveTexture, код ошибки: %d\n", src.AsCharPtr(), cmp_status);
            return false;
        }

        CMP_FreeMipSet(&MipSetIn);
        CMP_FreeMipSet(&MipSetCmp);

        Timing::Time after = timer.GetTime();
        this->logger->Print("Выполнено после: %f\n", after);
        return true;
    }
    return false;
}

}// пространство имен ToolkitUtil
