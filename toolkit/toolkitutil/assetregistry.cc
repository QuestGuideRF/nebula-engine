//------------------------------------------------------------------------------
//  assetregistry.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "assetregistry.h"
#include "io/ioserver.h"
#include "io/uri.h"
#include "io/stream.h"
#include "io/binarywriter.h"
#include "io/binaryreader.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
AssetRegistry::AssetRegistry() :
    numScannedFiles(0),
    numAddedFiles(0),
    numChangedFiles(0),
    numIdenticalFiles(0),
    numDeletedFiles(0),
    numTouchedFiles(0),
    numErrorFiles(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
    This method loads the registry file (if it exists), updates it by
    scanning the actual file system, and saves back the registry file.
*/
bool
AssetRegistry::UpdateLocal(Logger& logger)
{
    IoServer* ioServer = IoServer::Instance();
    this->updateLocalTimer.Start();

    // first try to load registry file (may not exist yet)
    if (!this->rootPath.IsValid())
    {
        logger.Error("Корневой путь не указан!\n");
        return false;
    }
    if (!this->registryPath.IsValid())
    {
        logger.Error("Файл реестра активов не установлен!\n");
        return false;
    }
    URI registryURI = this->rootPath + this->registryPath;
    if (ioServer->FileExists(registryURI))
    {
        logger.Print(">> Загрузка файла локального реестра активов...\n");
        if (!this->LoadRegistryFile(logger, registryURI))
        {
            return false;
        }
    }
    logger.Print("%d файлов в файле реестра\n", this->registry.Size());

    // обновить реестр путем анализа каталогов активов
    this->numScannedFiles = 0;
    this->numAddedFiles = 0;
    this->numChangedFiles = 0;
    this->numDeletedFiles = 0;
    this->numIdenticalFiles = 0;
    this->numTouchedFiles = 0;
    this->numErrorFiles = 0;
    logger.Print(">> Сканирование локальных файлов ресурсов...\n");
    if (!this->UpdateRegistry(logger))
    {
        return false;
    }

    // удалить файлы, которых больше нет из реестра
    // (все файлы, которые все еще находятся в состоянии «Неизвестно» после
    // сканирование локального файла)
    this->RemoveObsoleteFilesFromRegistry(logger);

    // сохранить реестр
    logger.Print(">> Сохранить файл реестра...\n");
    if (!this->SaveRegistryFile(logger, registryURI))
    {
        return false;
    }
    this->updateLocalTimer.Stop();

    // распечатать сводку
    logger.Print("\nПОСЛЕ ЛОКАЛЬНОГО СКАНИРОВАНИЯ: %d идентичен, %d новый, %d изменен, %d удален.\n", this->numIdenticalFiles, this->numAddedFiles, this->numChangedFiles, this->numDeletedFiles);
    logger.Print("Время локального обновления: %.3f секунд\n", this->updateLocalTimer.GetTime());

    return true;
}

//------------------------------------------------------------------------------
/**
    Этот метод загружает только файл реестра (обычно с удаленного компьютера).
    каталог сервера сборки).
*/
bool
AssetRegistry::UpdateRemote(Logger& logger)
{
    IoServer* ioServer = IoServer::Instance();

    // first try to load registry file (may not exist yet)
    if (!this->rootPath.IsValid())
    {
        logger.Error("Корневой путь не указан!\n");
        return false;
    }
    if (!this->registryPath.IsValid())
    {
        logger.Error("Файл реестра активов не установлен!\n");
        return false;
    }
    URI registryURI = this->rootPath + this->registryPath;
    if (ioServer->FileExists(registryURI))
    {
        logger.Print(">> Загрузка файла реестра удаленных активов...\n");
        if (this->LoadRegistryFile(logger, registryURI))
        {
            return true;
        }
    }
    else
    {
        logger.Error("Не удалось загрузить файл удаленного реестра: %s!\n", registryURI.AsString().AsCharPtr());
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    This iterates through all asset directories and builds a registry
    with all files. If a file already exists in the registry it's file time
    is checked against the value in the registry, and if it is different
    the CRC checksum will be re-computed.
*/
bool
AssetRegistry::UpdateRegistry(Logger& logger)
{
    bool retval = true;
    IoServer* ioServer = IoServer::Instance();

    // for each asset directory
    IndexT i;
    for (i = 0; i < this->directories.Size(); i++)
    {
        if (ioServer->DirectoryExists(this->rootPath + this->directories[i]))
        {
            retval &= this->RecurseUpdateRegistry(logger, this->directories[i]);
        }
    }
    return retval;
}

//------------------------------------------------------------------------------
/**
    Iterate over files in directory and add them to the registry (if 
    they don't exist yet), then recurse into sub directories.
*/
bool
AssetRegistry::RecurseUpdateRegistry(Logger& logger, const String& dir)
{
    IoServer* ioServer = IoServer::Instance();
    URI dirURI = this->rootPath + dir;
    if (!ioServer->DirectoryExists(dirURI))
    {
        logger.Error("Каталог ресурсов не найден: '%s'!\n", dir.AsCharPtr());
        return false;
    }
    // logger.Print("> %s\n", dir.AsCharPtr());

    // перебирать файлы
    Array<String> files = ioServer->ListFiles(dirURI, "*");
    IndexT i;
    for (i = 0; i < files.Size(); i++)
    {
        this->numScannedFiles++;
        String filePath;
        filePath.Format("%s/%s", dir.AsCharPtr(), files[i].AsCharPtr());
        StringAtom filePathAtom(filePath);
        if (!this->registry.Contains(filePathAtom))
        {
            this->AddAssetFile(logger, filePathAtom);
        }
        else
        {
            this->UpdateAssetFile(logger, filePathAtom);
        }
    }

    // войти в подкаталоги
    Array<String> subDirs = ioServer->ListDirectories(dirURI, "*");
    for (i = 0; i < subDirs.Size(); i++)
    {
        if ((subDirs[i] != "CVS") && (subDirs[i] != ".svn"))
        {
            String subDirPath;
            subDirPath.Format("%s/%s", dir.AsCharPtr(), subDirs[i].AsCharPtr());
            this->RecurseUpdateRegistry(logger, subDirPath);
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    This method is called after the local asset files have been scanned
    and the registry has been updated. Any file in the registry which
    still has the Unknown state no longer exists in the file system, thus
    this method will remove it from the registry.
*/
void
AssetRegistry::RemoveObsoleteFilesFromRegistry(Logger& logger)
{
    IndexT i;
    for (i = this->registry.Size() - 1; i != InvalidIndex; i--)
    {
        if (this->registry.ValueAtIndex(i).GetState() == AssetFile::Unknown)
        {
            this->numDeletedFiles++;
            this->registry.EraseAtIndex(i);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Add a new file to the file registry. This will read the file modification
    time and compute a crc checksum from the file.
*/
void
AssetRegistry::AddAssetFile(Logger& logger, const StringAtom& filePath)
{
    String fullPath = this->rootPath + filePath.Value();

    IoServer* ioServer = IoServer::Instance();
    AssetFile assetFile;
    assetFile.SetPath(filePath);
    assetFile.SetTimeStamp(ioServer->GetFileWriteTime(fullPath));
    assetFile.SetChecksum(ioServer->ComputeFileCrc(fullPath));
    assetFile.SetState(AssetFile::New);
    this->registry.Add(filePath, assetFile);
    this->numAddedFiles++;
}

//------------------------------------------------------------------------------
/**
    Update an existing entry in the registry. This first checks whether the
    time stamp of the actual file is newer then the time stamp recorded in 
    the registry (it should never be older!), and if the actual time stamp
    is newer, will update the crc checksum.
*/
void
AssetRegistry::UpdateAssetFile(Logger& logger, const StringAtom& filePath)
{
    IoServer* ioServer = IoServer::Instance();
    AssetFile& assetFile = this->registry[filePath];
    n_assert(assetFile.GetPath() == filePath);
    
    String fullPath = this->rootPath + filePath.Value();

    FileTime fileTime = ioServer->GetFileWriteTime(fullPath);
    if (fileTime != assetFile.GetTimeStamp())
    {
        // необходимо обновить контрольную сумму crc
        assetFile.SetChecksum(ioServer->ComputeFileCrc(fullPath));
        assetFile.SetTimeStamp(fileTime);
        assetFile.SetState(AssetFile::Changed);
        this->numChangedFiles++;
    }
    else
    {
        assetFile.SetState(AssetFile::Unchanged);
        this->numIdenticalFiles++;
    }
}

//------------------------------------------------------------------------------
/**
    Загрузите файл реестра.
*/
bool
AssetRegistry::LoadRegistryFile(Logger& logger, const URI& uri)
{
    n_assert(this->registry.IsEmpty());

    IoServer* ioServer = IoServer::Instance();
    Ptr<Stream> stream = ioServer->CreateStream(uri);
    Ptr<BinaryReader> reader = BinaryReader::Create();
    reader->SetMemoryMappingEnabled(true);
    reader->SetStream(stream);
    if (reader->Open())
    {
        // прочитать заголовок
        FourCC magic = reader->ReadUInt();
        if (magic != FourCC('NREG'))
        {
            logger.Error("Неверный формат файла реестра (%s)\n", uri.AsString().AsCharPtr());
            return false;
        }
        SizeT numItems = reader->ReadUInt();
        if (numItems > 0)
        {
            this->registry.Reserve(numItems);
            this->registry.BeginBulkAdd();
            IndexT i;
            for (i = 0; i < numItems; i++)
            {
                FileTime timeStamp;
                AssetFile assetFile;
                assetFile.SetPath(reader->ReadString());        
                uint highbits = reader->ReadUInt();
                uint lowbits = reader->ReadUInt();
                timeStamp.SetBits(lowbits, highbits);
                assetFile.SetTimeStamp(timeStamp);
                assetFile.SetChecksum(reader->ReadUInt());
                this->registry.Add(assetFile.GetPath(), assetFile);
            }
            this->registry.EndBulkAdd();
        }
        reader->Close();
        return true;
    }
    else
    {
        logger.Error("Не удалось открыть файл реестра «%s» для чтения!\n", uri.AsString().AsCharPtr());
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Открытый метод для сохранения файла реестра обратно.
*/
bool
AssetRegistry::SaveRegistry(Logger& logger)
{
    URI registryURI = this->rootPath + this->registryPath;
    return this->SaveRegistryFile(logger, registryURI);
}

//------------------------------------------------------------------------------
/**
    Save the registry file (private).
*/
bool
AssetRegistry::SaveRegistryFile(Logger& logger, const URI& uri)
{
    IoServer* ioServer = IoServer::Instance();
    Ptr<Stream> stream = ioServer->CreateStream(uri);
    Ptr<BinaryWriter> writer = BinaryWriter::Create();
    writer->SetStream(stream);
    if (writer->Open())
    {
        writer->WriteUInt(FourCC('NREG').AsUInt());
        writer->WriteInt(this->registry.Size());
        IndexT i;
        for (i = 0; i < this->registry.Size(); i++)
        {
            const AssetFile& assetFile = this->registry.ValueAtIndex(i);
            writer->WriteString(assetFile.GetPath().Value());
            writer->WriteUInt(assetFile.GetTimeStamp().GetHighBits());
            writer->WriteUInt(assetFile.GetTimeStamp().GetLowBits());
            writer->WriteUInt(assetFile.GetChecksum());
        }
        writer->Close();
        return true;
    }
    else
    {
        logger.Error("Не удалось открыть файл реестра «%s» для записи!\n", uri.AsString().AsCharPtr());
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    This method takes a local and a remote asset registry (represents the
    asset files on a local client, and on a remote build server), and builds
    a difference map, which contains for each local asset file a state
    (New, Deleted, Changed). Identical files will not be stored in the
    registry.
*/
bool
AssetRegistry::BuildDifference(Logger& logger, AssetRegistry& localRegistry, const AssetRegistry& remoteRegistry)
{
    n_assert(this->registry.IsEmpty());
    logger.Print(">> Различие локальных и удаленных ресурсов...\n");

    // сбросить счетчики статистики
    this->numScannedFiles = 0;
    this->numAddedFiles = 0;
    this->numChangedFiles = 0;
    this->numIdenticalFiles = 0;
    this->numDeletedFiles = 0;
    this->numTouchedFiles = 0;
    this->numErrorFiles = 0;

    // отслеживать состояния локального реестра
    FixedArray<AssetFile::State> localStates(localRegistry.registry.Size(), AssetFile::Unknown);

    // for each file in the remote registry...
    this->registry.Reserve(remoteRegistry.registry.Size());
    this->registry.BeginBulkAdd();
    IndexT remoteIndex;
    for (remoteIndex = 0; remoteIndex < remoteRegistry.registry.Size(); remoteIndex++)
    {
        const AssetFile& remoteAsset = remoteRegistry.registry.ValueAtIndex(remoteIndex);
        AssetFile::State assetState = AssetFile::Unknown;

        // найти тот же актив в локальном реестре
        IndexT localIndex = localRegistry.registry.FindIndex(remoteAsset.GetPath());
        if (InvalidIndex == localIndex)
        {
            // новый файл на сервере сборки, который еще не существует локально
            assetState = AssetFile::New;
            this->numAddedFiles++;
        }
        else
        {
            // файл уже существует локально, но нам нужно проверить, существует ли 
            // содержимое изменилось, или версия сервера сборки на самом деле изменилась.
            // более поздняя, ​​чем наша версия (поскольку копирование происходит быстрее, чем
            // экспорт)
            AssetFile& localAsset = localRegistry.registry.ValueAtIndex(localIndex);
            if (localAsset.GetChecksum() != remoteAsset.GetChecksum())
            {
                // локальный файл должен быть обновлен
                assetState = AssetFile::Changed;
                localAsset.SetTimeStamp(remoteAsset.GetTimeStamp());
                localAsset.SetChecksum(remoteAsset.GetChecksum());
                this->numChangedFiles++;
            }
            else if (localAsset.GetTimeStamp() < remoteAsset.GetTimeStamp())
            {
                // необходимо обновить только местную отметку времени
                assetState = AssetFile::Touched;
                localAsset.SetTimeStamp(remoteAsset.GetTimeStamp());
                this->numTouchedFiles++;
            }
            else
            {
                // локальный файл обновлен
                assetState = AssetFile::Unchanged;
                this->numIdenticalFiles++;
            }
            // обновить массив localStates, который отслеживает действительные активы
            localStates[localIndex] = assetState;
        }

        // add a new item to our registry (only if the file is new or changed)
        if ((AssetFile::Changed == assetState) || (AssetFile::New == assetState) || (AssetFile::Touched == assetState))
        {
            AssetFile diffAsset;
            diffAsset.SetPath(remoteAsset.GetPath());
            diffAsset.SetTimeStamp(remoteAsset.GetTimeStamp());
            diffAsset.SetChecksum(remoteAsset.GetChecksum());
            diffAsset.SetState(assetState);
            this->registry.Add(diffAsset.GetPath(), diffAsset);
        }
    }

    // теперь нам нужно просканировать массив localStates на предмет ресурсов, которых больше не существует.
    // в удаленном реестре
    IndexT i;
    for (i = 0; i < localStates.Size(); i++)
    {
        if (AssetFile::Unknown == localStates[i])
        {
            // этот актив больше не существует, необходимо записать элемент «Удалить» в наш реестр
            AssetFile deadAsset;
            deadAsset.SetPath(localRegistry.registry.ValueAtIndex(i).GetPath());
            deadAsset.SetState(AssetFile::Deleted);
            this->registry.Add(deadAsset.GetPath(), deadAsset);
            this->numDeletedFiles++;
        }
    }
    this->registry.EndBulkAdd();

    // распечатать статистику
    logger.Print("\nПОСЛЕ РАЗЛИЧИЯ: %d идентичен, %d изменен, %d новый, %d изменен, %d удален.\n", this->numIdenticalFiles, this->numTouchedFiles, this->numAddedFiles, this->numChangedFiles, this->numDeletedFiles);
    return true;
}

} // пространство имен ToolkitUtil
