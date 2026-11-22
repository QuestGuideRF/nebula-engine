//------------------------------------------------------------------------------
//  nvtttextureconversionjob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"

#include "nvtttextureconversionjob.h"
#include "io/uri.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "timing/timer.h"

#include <nvtt/nvtt.h>
#include <nvtt/TaskDispatcher.h>

#include <nvimage/Image.h> 
#include <nvimage/ImageIO.h>
#include <nvimage/FloatImage.h>
#include <nvimage/Filter.h>
#include <nvimage/DirectDrawSurface.h>

#include <nvcore/Ptr.h> // AutoPtr
#include <nvcore/StrLib.h> // Path
#include <nvcore/StdStream.h>
#include <nvcore/FileSystem.h>
#include <nvcore/Timer.h>
namespace ToolkitUtil
{
using namespace IO;
using namespace Util;


//------------------------------------------------------------------------------
/**
*/
static nvtt::Format TextureAttrToNVTT(ToolkitUtil::TextureAttrs::PixelFormat format)
{
    switch(format)
    {
        case TextureAttrs::DXT1C:
            return nvtt::Format_DXT1;
        case TextureAttrs::DXT1A:
            return nvtt::Format_DXT1a;
        case TextureAttrs::DXT3:
            return nvtt::Format_DXT3;
        case TextureAttrs::DXT5:
            return nvtt::Format_DXT5;
        case TextureAttrs::DXT5NM:
            return nvtt::Format_DXT5n;
        case TextureAttrs::U8888:
            return nvtt::Format_RGBA;
        case TextureAttrs::U888:
            return nvtt::Format_RGB;
        case TextureAttrs::BC6:
            return nvtt::Format_BC6;
        case TextureAttrs::BC7:
            return nvtt::Format_BC7;
        default:
            n_error("неподдерживаемый формат сжатия текстур");
            return nvtt::Format_RGB;
    }
        
}

//------------------------------------------------------------------------------
/**
*/
NVTTTextureConversionJob::NVTTTextureConversionJob()
{
    this->SetDstFileExtension("dds");
}

//------------------------------------------------------------------------------
/**
    Запустите процесс конвертации. Возвращает false, если задание завершилось немедленно.   
*/
bool
NVTTTextureConversionJob::Convert()
{
    n_assert(0 != this->logger);
    if (TextureConversionJob::Convert())
    {  
        URI srcPathUri(this->srcPath);
        URI dstPathUri(this->dstPath);
        URI tmpDirUri(this->tmpDir);
        
        String src = srcPathUri.LocalPath();                      
        if(src.CheckFileExtension("dds"))
        {
            // Поверхность загрузки.
            nv::DirectDrawSurface dds(src.AsCharPtr());
            if (!dds.isValid())
            {
                this->logger->Warning("Файл «%s» не является допустимым файлом DDS.\n", src.AsCharPtr());
                return false;
            }

            if (!dds.isSupported() || dds.isTexture3D())
            {
                this->logger->Warning("Файл «%s» не является поддерживаемым файлом DDS.\n", src.AsCharPtr());            
                return false;
            }
            IoServer::Instance()->CopyFile(this->srcPath, this->dstPath);
            
            return true;            
        }
        
        // мы делаем все в плавающем режиме, чтобы можно было изменить размер
        nv::FloatImage *image;        
        nv::AutoPtr<nv::Image> outputImage;
        nvtt::InputOptions inputOptions;
        
        Timing::Timer timer;
        timer.Start();      
        this->logger->Print("Обработка: %s\n", src.AsCharPtr());

        nvtt::OutputOptions outputOptions;
        if (src.CheckFileExtension("exr") || src.CheckFileExtension("hdr"))
        {
            image = nv::ImageIO::loadFloat(src.AsCharPtr());
            outputOptions.setContainer(nvtt::Container_DDS10);

            if (image == NULL)
            {
                this->logger->Warning("Файл «%s» не является поддерживаемым типом изображения.\n", src.AsCharPtr());                    
                return false;
            }                        
        }
        else
        {
            // Обычное изображение.            
            nv::Image rawimage;
            if (!rawimage.load(src.AsCharPtr()))
            {
                this->logger->Warning("Файл «%s» не является поддерживаемым типом изображения.\n", src.AsCharPtr());                
                return false;
            }
            
            image = new nv::FloatImage(&rawimage);            
        }
        
        bool isDXT5NormalMap = false;
        const TextureAttrs& attrs = this->textureAttrs;
        
        ToolkitUtil::TextureAttrs::PixelFormat targetformat;
        float gamma = 1.0f;
        if ((attrs.GetRGBPixelFormat() == TextureAttrs::DXT5NM) ||
            (attrs.GetRGBAPixelFormat() == TextureAttrs::DXT5NM) ||
            (String::MatchPattern(this->srcPath, "*норма.*"))||
            (String::MatchPattern(this->srcPath, "*нормальный.*"))||
            (String::MatchPattern(this->srcPath, "*ударяться.*")))
        {
            inputOptions.setNormalMap(true);
            inputOptions.setConvertToNormalMap(false);
            inputOptions.setGamma(1.0f, 1.0f);
            inputOptions.setNormalizeMipmaps(true);   
            if (attrs.GetFlipNormalY())
            {
                image->scaleBias(1, 1, -1, 1);
                //изображение->flipY();
            }
            isDXT5NormalMap = true;
            targetformat = TextureAttrs::DXT5NM;            
        }
        else
        {
            if ((String::MatchPattern(this->srcPath, "*мат.*")) ||
                (String::MatchPattern(this->srcPath, "*материал.*")))
            {
                targetformat = TextureAttrs::DXT5;
            }
            else if (image->componentCount() > 3)
            {
                targetformat = attrs.GetRGBAPixelFormat();
            }
            else
            {
                targetformat = attrs.GetRGBPixelFormat();
            }

            
            if (attrs.GetColorSpace() == TextureAttrs::Linear)
            {
                gamma = 1.0f;
                inputOptions.setGamma(1.0f, 1.0f);
            }
            else if (attrs.GetColorSpace() == TextureAttrs::sRGB)
            {
                gamma = 2.2f;
                inputOptions.setGamma(2.2f, 2.2f);
                outputOptions.setContainer(nvtt::Container_DDS10);
            }
        }
        
        nvtt::CompressionOptions compressionOptions;
        compressionOptions.setFormat(TextureAttrToNVTT(targetformat));
        
        if (attrs.GetQuality() == TextureAttrs::Normal)  compressionOptions.setQuality(nvtt::Quality_Normal);
        else if (attrs.GetQuality() == ToolkitUtil::TextureAttrs::High)     compressionOptions.setQuality(nvtt::Quality_Production);
        else if (attrs.GetQuality() == ToolkitUtil::TextureAttrs::Low)      compressionOptions.setQuality(nvtt::Quality_Fastest);
        
        if(targetformat == TextureAttrs::DXT3)
        {
            // Дизеринг альфа при использовании BC2.
            compressionOptions.setQuantization(false, true, false);
        }
        else if (targetformat == TextureAttrs::DXT1A)
        {
            // Двоичная альфа при использовании BC1a.
            compressionOptions.setQuantization(false, true, true, 127);
        }
        
        bool needResize = false;
        int width = image->width();
        int height = image->height();
        if (attrs.GetMaxWidth() < width)
        {
            width = attrs.GetMaxWidth();
            needResize = true;
        }
        if (attrs.GetMaxHeight() < height)
        {
            height = attrs.GetMaxHeight();
            needResize = true;
        }
        
        if (needResize)
        {
            Timing::Time startResize = timer.GetTime();
            this->logger->Print("Изменение размера до %d %d с помощью %s ....", width, height, ToolkitUtil::TextureAttrs::FilterToString(attrs.GetScaleFilter()).AsCharPtr());
            nv::AutoPtr<nv::Filter> filter;
            if (attrs.GetScaleFilter() == ToolkitUtil::TextureAttrs::Kaiser)
            {
                filter = new nv::KaiserFilter(3);
                ((nv::KaiserFilter *)filter.ptr())->setParameters(4.0f, 1.0f);
            }
            else if (attrs.GetScaleFilter() == ToolkitUtil::TextureAttrs::Box)
            {
                filter = new nv::BoxFilter();
            }
            else if (attrs.GetScaleFilter() == ToolkitUtil::TextureAttrs::Triangle)
            {
                filter = new nv::TriangleFilter();
            }
            else if (attrs.GetScaleFilter() == ToolkitUtil::TextureAttrs::Quadrat)
            {
                filter = new nv::QuadraticFilter();
            }
            else if (attrs.GetScaleFilter() == ToolkitUtil::TextureAttrs::Cubic)
            {
                filter = new nv::CubicFilter();
            }
            else
            {
                filter = new nv::LanczosFilter();
            }           

            image->toLinear(0, 3, gamma);
            nv::AutoPtr<nv::FloatImage> fresult(image->resize(*filter, width, height, nv::FloatImage::WrapMode_Clamp));
            outputImage = fresult->createImageGammaCorrect(gamma);            
            delete image;
            Timing::Time diff = timer.GetTime() - startResize;
            this->logger->Print("сделано после %f\n",diff);
        }
        else
        {
            outputImage = image->createImage(0, image->m_componentCount);
            delete image;
        }
                    
        inputOptions.setTextureLayout(nvtt::TextureType_2D, width, height);
        inputOptions.setMipmapData(outputImage->pixels(),width,height);
        inputOptions.setRoundMode(nvtt::RoundMode_ToPreviousPowerOfTwo);
        
        if (!attrs.GetGenMipMaps()) inputOptions.setMipmapGeneration(false);
        
        if (attrs.GetMipMapFilter() == ToolkitUtil::TextureAttrs::Kaiser)           inputOptions.setMipmapFilter(nvtt::MipmapFilter_Kaiser);
        else if (attrs.GetMipMapFilter() == ToolkitUtil::TextureAttrs::Triangle)    inputOptions.setMipmapFilter(nvtt::MipmapFilter_Triangle);
        else if (attrs.GetMipMapFilter() == ToolkitUtil::TextureAttrs::Box)         inputOptions.setMipmapFilter(nvtt::MipmapFilter_Box);
        else                                                                        inputOptions.setMipmapFilter(nvtt::MipmapFilter_Box);
        
        outputOptions.setFileName(dstPathUri.LocalPath().AsCharPtr());
        if (!isDXT5NormalMap)   outputOptions.setSrgbFlag(attrs.GetColorSpace() == TextureAttrs::sRGB);

        Timing::Time beforeCompress = timer.GetTime();
        this->logger->Print("Сжатие...");
        nvtt::Context context;
        nv::AutoPtr<nvtt::ParallelTaskDispatcher> taskDispatcher = new nvtt::ParallelTaskDispatcher();
        context.setTaskDispatcher(taskDispatcher.ptr());
        if (!context.process(inputOptions, compressionOptions, outputOptions))
        {
            this->logger->Print("Не удалось!\n");
            return false;
        }
        Timing::Time after = timer.GetTime();
        this->logger->Print("Выполнено после %f, общее время: %f\n", after - beforeCompress, after);
        return true;         
    }
    return true;
}

}// пространство имен ToolkitUtil