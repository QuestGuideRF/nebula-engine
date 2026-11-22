//------------------------------------------------------------------------------
//  rcswrapper.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "rcswrapper.h"
#include "applauncher.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/memorystream.h"
#include "io/xmlreader.h"
#include "io/textreader.h"

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
RCSWrapper::RCSWrapper()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
RCSWrapper::~RCSWrapper()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    При этом временный файл конфигурации будет записан в «temp:synctool3/». Конфигурация
    file инструктирует SVN установить временные метки файла на время фиксации.
*/
void
RCSWrapper::WriteTempSVNConfigFiles()
{
    IoServer* ioServer = IoServer::Instance();
    ioServer->CreateDirectory("temp:synctool3");
    Ptr<Stream> stream = ioServer->CreateStream("temp:synctool3/config");
    Ptr<TextWriter> textWriter = TextWriter::Create();
    textWriter->SetStream(stream);
    if (textWriter->Open())
    {
        textWriter->WriteLine("[аутент.]");
        textWriter->WriteLine("хранить пароли = нет");
        textWriter->WriteLine("store-auth-creds = нет");
        textWriter->WriteLine("[помощники]");
        textWriter->WriteLine("[туннели]");
        textWriter->WriteLine("[разное]");
        textWriter->WriteLine("использование-фиксации-раз = да");
        textWriter->WriteLine("[автореквизит]");
        textWriter->WriteLine("*.xls = svn:требуется блокировка");
        textWriter->WriteLine("*.xlsx = svn:требуется блокировка");
        textWriter->WriteLine("*.doc = svn:требуется блокировка");
        textWriter->WriteLine("*.docx = svn:needs-lock");
        textWriter->WriteLine("*.jpg = svn:требуется блокировка");
        textWriter->WriteLine("*.psd = svn:needs-lock");
        textWriter->WriteLine("*.mb = svn:требуется блокировка");
        textWriter->WriteLine("*.xml = svn:требуется блокировка");
        textWriter->WriteLine("*.exe = svn:требуется блокировка");
        textWriter->WriteLine("*.ppt = svn:требуется блокировка");
        textWriter->WriteLine("*.mpp = svn:требуется блокировка");
        textWriter->WriteLine("*.pdf = svn:требуется блокировка");
        textWriter->WriteLine("*.mmap = svn:требуется блокировка");
        textWriter->WriteLine("*.pst = svn:needs-lock");
        textWriter->WriteLine("*.msg = svn:требуется блокировка");
        textWriter->WriteLine("*.rtf = svn:требуется блокировка");
        textWriter->WriteLine("*.cab = svn:требуется блокировка");
        textWriter->WriteLine("*.dll = svn:требуется блокировка");
        textWriter->WriteLine("*.bat = svn:требуется блокировка");
        textWriter->WriteLine("*.bmp = svn:требуется блокировка");
        textWriter->WriteLine("*.ico = svn:требуется блокировка");
        textWriter->WriteLine("*.8bi = svn:требуется блокировка");
        textWriter->WriteLine("*.8bf = svn:требуется блокировка");
        textWriter->WriteLine("*.xsd = svn:требуется блокировка");
        textWriter->WriteLine("*.tga = svn:требуется блокировка");
        textWriter->WriteLine("*.dds = svn:needs-lock");
        textWriter->Close();

        ioServer->CopyFile("temp:synctool3/config", "temp:synctool3/config.txt");
    }

    // также необходимо записать файлы аутентификации пользователя
    this->CopyDirectory("appdata:Subversion/auth/svn.simple", "temp:synctool3/auth/svn.simple");
    this->CopyDirectory("appdata:Subversion/auth/svn.ssl.server", "temp:synctool3/auth/svn.ssl.server");
    this->CopyDirectory("appdata:Subversion/auth/svn.username", "temp:synctool3/auth/svn.username");
}

//------------------------------------------------------------------------------
/**
    Вспомогательный метод для копирования содержимого каталога. Используется при записи временных 
    Данные конфигурации Subversion.
*/
void
RCSWrapper::CopyDirectory(const String& from, const String& to)
{
    IoServer* ioServer = IoServer::Instance();
    URI fromUri(from);
    URI toUri(to);
    if (ioServer->DirectoryExists(fromUri))
    {
        if (ioServer->CreateDirectory(toUri))
        {
            Array<String> files = ioServer->ListFiles(fromUri, "*");
            IndexT i;
            for (i = 0; i < files.Size(); i++)
            {
                String fromPath = from + "/" + files[i];
                String toPath = to + "/" + files[i];
                ioServer->CopyFile(URI(fromPath), URI(toPath));
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::Update(Logger& logger)
{
    // напишите файл версии SVN, который включает «время использования-фиксации»
    this->WriteTempSVNConfigFiles();

    // сначала запустите обновление отметки времени фиксации, это установит 
    // временная метка обновленных файлов до времени фиксации
    IndexT i;
    for (i = 0; i < this->commitTimeStampDirs.Size(); i++)
    {
        if (!this->UpdateDirectory(logger, this->commitTimeStampDirs[i], true))
        {
            return false;
        }
    }

    // затем запустите обновления меток времени обновления, это установит
    // Error 500 (Server Error)!!1500.That’s an error.There was an error. Please try again later.That’s all we know.
    for (i = 0; i < this->updateTimeStampDirs.Size(); i++)
    {
        if (!this->UpdateDirectory(logger, this->updateTimeStampDirs[i], false))
        {
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::UpdateDirectory(Logger& logger, const String& directory, bool commitTimeStampMode)
{
    logger.Print("Обновление %s...\n", directory.AsCharPtr());

    // проверьте, находится ли каталог под контролем SVN
    if (!this->IsSVNDirectory(logger, directory))
    {
        return false;
    }

    // make sure the Subversion command line tool has been set and exists
    if (!this->IsSVNCommandLineConfigured(logger))
    {
        return false;
    }

    // запустить инструмент командной строки SVN
    AppLauncher appLauncher;
    appLauncher.SetWorkingDirectory(directory);
    String svnArgs;
    if (commitTimeStampMode)
    {
        String resolvedConfigDir = AssignRegistry::Instance()->ResolveAssigns("temp:synctool3").LocalPath();
        svnArgs.Format("update --config-dir %s --неинтерактивный", resolvedConfigDir.AsCharPtr());
    }
    else
    {
        svnArgs = "обновление --неинтерактивное";
    }
    appLauncher.SetExecutable(this->svnToolLocation);
    appLauncher.SetArguments(svnArgs);
    
    Ptr<MemoryStream> memoryStream = MemoryStream::Create();
    appLauncher.SetStderrCaptureStream(memoryStream.upcast<Stream>());

    logger.Print(("RCSWrapper::UpdateDirectory: " + this->svnToolLocation + " " + svnArgs + "\n").AsCharPtr());

    if (!appLauncher.LaunchWait())
    {
        logger.Error("Не удалось запустить инструмент SVN!\n");
        return false;
    }
    
    // анализировать вывод на наличие ошибок
    Ptr<IO::TextReader> reader = IO::TextReader::Create();
    reader->SetStream(memoryStream.upcast<Stream>());
    reader->Open();
    String line;
    bool foundError = false;
    while (!reader->Eof())
    {
        line = reader->ReadLine();
        if(line.FindStringIndex("svn:")!=InvalidIndex)
        {
            foundError = true;
        }
        n_printf("%s\n",line.AsCharPtr());
    }

    reader->Close();

    if(foundError)
    {
        logger.Error("При обновлении каталога возникли ошибки!\nПожалуйста, попробуйте выполнить очистку.\n");
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::GetModifiedFiles(Logger& logger, const String& directory, Array<String>& outFiles) const
{
    // проверьте, находится ли каталог под контролем SVN
    if (!this->IsSVNDirectory(logger, directory))
    {
        return false;
    }

    // make sure the Subversion command line tool has been set and exists
    if (!this->IsSVNCommandLineConfigured(logger))
    {
        return false;
    }
    
    // запустить инструмент командной строки SVN, перенаправить вывод в поток памяти
    AppLauncher appLauncher;
    appLauncher.SetWorkingDirectory(directory);
    const String& svnArgs = "статус --xml --неинтерактивный";
    appLauncher.SetExecutable(this->svnToolLocation);
    appLauncher.SetArguments(svnArgs);
    
    Ptr<MemoryStream> memoryStream = MemoryStream::Create();
    appLauncher.SetStdoutCaptureStream(memoryStream.upcast<Stream>());

    logger.Print(("RCSWrapper::GetModifiedFiles: " + this->svnToolLocation + " " + svnArgs + "\n").AsCharPtr());
    if (!appLauncher.LaunchWait())
    {
        logger.Error("Не удалось запустить инструмент SVN!\n");
        return false;
    }
    
    if(!this->ParseSVNStatusOutput(memoryStream.upcast<Stream>(), directory, outFiles))
    {
        logger.Error("Не удалось проанализировать вывод статуса SVN!\n");
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::IsSVNDirectory(Logger& logger, const String& directory) const
{
    IoServer* ioServer = IoServer::Instance();

    if (!ioServer->DirectoryExists(directory + "/.svn"))
    {
        logger.Error("Каталог не находится под контролем SVN или CVS: %s\n", directory.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::IsSVNCommandLineConfigured(Logger& logger) const
{
    IoServer* ioServer = IoServer::Instance();
    
    if (!this->svnToolLocation.IsValid())
    {
        logger.Error("Инструмент командной строки SVN не установлен!\n");
        return false;
    }
    if (!ioServer->FileExists(this->svnToolLocation))
    {
        logger.Error("Инструмент командной строки SVN не найден: '%s'!\n", this->svnToolLocation.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
RCSWrapper::ParseSVNStatusOutput(const Ptr<Stream>& stream, const String& directory, Array<String>& outFiles) const
{
    Ptr<XmlReader> xmlReader = XmlReader::Create();
    xmlReader->SetStream(stream);
    // проанализировать выходные данные инструмента командной строки SVN на предмет измененных файлов в рабочей копии, добавить их в выходной массив
    if (xmlReader->Open())
    {
        if(xmlReader->SetToFirstChild("target"))
        {
            if(xmlReader->SetToFirstChild("entry"))
            {
                do
                {
                    String file = xmlReader->GetString("path");
                    if(xmlReader->SetToFirstChild("wc-status"))
                    {
                        if("modified" == xmlReader->GetString("item"))
                        {
                            outFiles.Append(file);
                        }
                        xmlReader->SetToParent();
                    }
                } while(xmlReader->SetToNextChild("entry"));
            }
        }
        xmlReader->Close();
        return true;
    }
    return false;
}

} // пространство имен ToolkitUtil