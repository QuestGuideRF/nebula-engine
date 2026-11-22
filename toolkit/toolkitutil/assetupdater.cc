//------------------------------------------------------------------------------
//  assetupdater.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "assetupdater.h"
#include "io/ioserver.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
AssetUpdater::AssetUpdater()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
AssetUpdater::Update(Logger& logger, const AssetRegistry& diffSet)
{
    logger.Print(">> Синхронизация локальных файлов...\n");

    IoServer* ioServer = IoServer::Instance();
    if (this->localRootPath.IsEmpty())
    {
        logger.Error("Не задан локальный корневой путь!\n");
        return false;
    }
    if (this->remoteRootPath.IsEmpty())
    {
        logger.Error("Не задан удаленный корневой путь!\n");
        return false;
    }
    SizeT numFilesCopied = 0;
    SizeT numFilesDeleted = 0;
    SizeT numFilesTouched = 0;
    SizeT numErrors = 0;

    // for each entry in the difference set...
    IndexT i;
    for (i = 0; i < diffSet.GetNumAssets(); i++)
    {
        const AssetFile& curEntry = diffSet.GetAssetByIndex(i);
        URI localFileUri(this->localRootPath + curEntry.GetPath().AsString());
        URI localDirUri(this->localRootPath + curEntry.GetPath().AsString().ExtractDirName());
        URI remoteFileUri(this->remoteRootPath + curEntry.GetPath().AsString());

        if ((curEntry.GetState() == AssetFile::New) ||
            (curEntry.GetState() == AssetFile::Changed))
        {
            // нужно скопировать файл с сервера
            logger.Print("КОПИРОВАТЬ %s\n", curEntry.GetPath().Value());
            if (!ioServer->DirectoryExists(localDirUri))
            {
                ioServer->CreateDirectory(localDirUri);
            }
            if (ioServer->CopyFile(remoteFileUri, localFileUri))
            {
                // установить отметку времени локального файла на отметку времени удаленного файла
                ioServer->SetFileWriteTime(localFileUri, curEntry.GetTimeStamp());
                numFilesCopied++;
            }
            else
            {
                logger.Error("Не удалось скопировать файл «%s» с сервера сборки!\n", curEntry.GetPath().Value());
                numErrors++;
            }
        }
        else if (curEntry.GetState() == AssetFile::Touched)
        {
            // обновлять только местную отметку времени
            // logger.Print("TOUCH %s\n", curEntry.GetPath().Value().AsCharPtr());
            ioServer->SetFileWriteTime(localFileUri, curEntry.GetTimeStamp());
            numFilesTouched++;
        }
        else if (curEntry.GetState() == AssetFile::Deleted)
        {
            // нужно удалить локальную копию
            logger.Print("DELETE %s\n", curEntry.GetPath().Value());
            if (ioServer->DeleteFile(localFileUri))
            {
                numFilesDeleted++;
            }
            else
            {
                logger.Error("Не удалось удалить локальный файл «%s»!\n", curEntry.GetPath().Value());
                numErrors++;
            }
        }
    }    

    // распечатать сводку:
    logger.Print("\nПОСЛЕ СИНХРОНИЗАЦИИ: %d скопировано, %d коснулось, %d удалено, %d ошибок.\n", numFilesCopied, numFilesTouched, numFilesDeleted, numErrors);
    return true;
}

} // пространство имен ToolkitUtil
