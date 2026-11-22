//------------------------------------------------------------------------------
//  animbuildersaver.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "model/animutil/animbuildersaver.h"
#include "io/ioserver.h"
#include "coreanimation/naxfileformatstructs.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;
using namespace System;
using namespace CoreAnimation;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
bool
AnimBuilderSaver::Save(const URI& uri, const Util::Array<AnimBuilder>& animBuilders, Platform::Code platform)
{
    // убедитесь, что целевой каталог существует
    IoServer::Instance()->CreateDirectory(uri.LocalPath().ExtractDirName());

    Ptr<Stream> stream = IoServer::Instance()->CreateStream(uri);
    stream->SetAccessMode(Stream::WriteAccess);
    if (stream->Open())
    {
        ByteOrder byteOrder(ByteOrder::Host, Platform::GetPlatformByteOrder(platform));
        AnimBuilderSaver::WriteHeader(stream, animBuilders, byteOrder);
        AnimBuilderSaver::WriteAnimations(stream, animBuilders, byteOrder);

        stream->Close();
        stream = nullptr;
        return true;
    }
    else
    {
        // не удалось открыть поток записи
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
AnimBuilderSaver::WriteHeader(const Ptr<Stream>& stream, const Util::Array<AnimBuilder>& animBuilders, const ByteOrder& byteOrder)
{
    // заголовок настройки
    Nax3Header nax3Header;
    nax3Header.magic         = byteOrder.Convert<uint>(NEBULA_NAX3_MAGICNUMBER);
    nax3Header.numAnimations = byteOrder.Convert(animBuilders.Size());

    // написать заголовок
    stream->Write(&nax3Header, sizeof(nax3Header));
}

//------------------------------------------------------------------------------
/**
*/
void 
AnimBuilderSaver::WriteAnimations(const Ptr<IO::Stream>& stream, const Util::Array<AnimBuilder>& animBuilders, const System::ByteOrder& byteOrder)
{
    for (auto& anim : animBuilders)
    {
        Nax3Anim nax3;
        nax3.numClips = anim.GetNumClips();
        nax3.numEvents = anim.events.Size();
        nax3.numCurves = anim.curves.Size();
        nax3.numKeys = anim.keys.Size();
        nax3.numIntervals = 0;
        for (const auto& curve : anim.curves)
        {
            nax3.numIntervals += curve.numKeys == 0 ? 0 : curve.numKeys - 1;
        }

        // написать заголовок
        stream->Write(&nax3, sizeof(nax3));

        Util::Array<Nax3Interval> intervals;

        for (const auto& curve : anim.curves)
        {
            // написать атрибуты кривой
            Nax3Curve nax3Curve;
            nax3Curve.firstIntervalOffset = intervals.Size();
            nax3Curve.numIntervals = curve.numKeys == 0 ? 0 : curve.numKeys - 1;
            nax3Curve.preInfinityType = curve.preInfinityType;
            nax3Curve.postInfinityType = curve.postInfinityType;
            nax3Curve.curveType = curve.curveType;

            byteOrder.ConvertInPlace(nax3Curve.firstIntervalOffset);
            byteOrder.ConvertInPlace(nax3Curve.numIntervals);

            // напиши в трансляцию
            stream->Write(&nax3Curve, sizeof(nax3Curve));            

            int stride = curve.curveType == CurveType::Rotation ? 4 : 3;

            // Создайте интервалы для клавиш
            for (IndexT i = 0; i < nax3Curve.numIntervals; i++)
            {
                Timing::Tick start = anim.keyTimes[curve.firstTimeOffset + i];
                Timing::Tick end = anim.keyTimes[curve.firstTimeOffset + i + 1];
                Nax3Interval interval;

                interval.start = byteOrder.Convert(start);
                interval.end = byteOrder.Convert(end);
                interval.key0 = byteOrder.Convert(curve.firstKeyOffset + i * stride);
                interval.key1 = byteOrder.Convert(curve.firstKeyOffset + (i + 1) * stride);
                interval.duration = 1 / float(interval.end - interval.start);

                intervals.Append(interval);
            }
        }

        for (const auto& animEvent : anim.events)
        {
            Nax3AnimEvent nax3AnimEvent;

            // проверьте ограничения на имена
            const String& eventName = animEvent.name.AsString();
            const String& categoryName = animEvent.category.AsString();
            if (eventName.Length() >= sizeof(nax3AnimEvent.name))
            {
                n_error("AnimBuilderSaver: имя события Anim слишком длинное! (file=%s, event=%s)\n",
                    stream->GetURI().LocalPath().AsCharPtr(),
                    eventName.AsCharPtr());
            }
            if (categoryName.Length() >= sizeof(nax3AnimEvent.category))
            {
                n_error("AnimBuilderServer: категория событий Anim слишком длинная! (файл=%s, событие=%s, категория=%s)\n",
                    stream->GetURI().LocalPath().AsCharPtr(),
                    eventName.AsCharPtr(),
                    categoryName.AsCharPtr());
            }

            // писать атрибуты событий
            nax3AnimEvent.keyIndex = byteOrder.Convert(animEvent.time);
            eventName.CopyToBuffer(&(nax3AnimEvent.name[0]), sizeof(nax3AnimEvent.name));
            categoryName.CopyToBuffer(&(nax3AnimEvent.category[0]), sizeof(nax3AnimEvent.category));

            // написать событие анимации для потоковой передачи
            stream->Write(&nax3AnimEvent, sizeof(nax3AnimEvent));
        }

        uint curveOffset = 0, eventOffset = 0, velocityCurveOffset = 0;
        SizeT numClips = anim.GetNumClips();
        IndexT clipIndex;
        for (clipIndex = 0; clipIndex < numClips; clipIndex++)
        {
            AnimBuilderClip& clip = anim.GetClipAtIndex(clipIndex);
            Nax3Clip nax3Clip;

            // проверьте ограничения на название клипа
            const String& clipName = clip.GetName().AsString();
            if (clipName.Length() >= sizeof(nax3Clip.name))
            {
                n_error("AnimBuilderSaver: имя клипа «%s» слишком длинное (%s)!\n", clipName.AsCharPtr(), stream->GetURI().LocalPath().AsCharPtr());
            }

            // записать атрибуты клипа
            clipName.CopyToBuffer(&(nax3Clip.name[0]), sizeof(nax3Clip.name));
            nax3Clip.firstCurve = byteOrder.Convert(curveOffset);
            nax3Clip.firstEvent = byteOrder.Convert(eventOffset);
            nax3Clip.firstVelocityCurve = byteOrder.Convert(velocityCurveOffset);
            nax3Clip.numCurves = byteOrder.Convert(clip.numCurves);
            nax3Clip.numEvents = byteOrder.Convert(clip.numEvents);
            nax3Clip.numVelocityCurves = byteOrder.Convert(clip.numVelocityCurves);
            nax3Clip.duration = byteOrder.Convert(clip.duration);

            curveOffset += clip.numCurves;
            eventOffset += clip.numEvents;
            velocityCurveOffset += clip.numVelocityCurves;

            // записать заголовок клипа в поток
            stream->Write(&nax3Clip, sizeof(nax3Clip));
        }

        for (const auto& interval : intervals)
        {
            stream->Write(&interval, sizeof(Nax3Interval));
        }

        for (const float key : anim.keys)
        {
            float value = byteOrder.Convert(key);
            stream->Write(&value, sizeof(float));
        }
    }
}

} // пространство имен ToolkitUtil
