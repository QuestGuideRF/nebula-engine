//------------------------------------------------------------------------------
//  db/dbserver.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/dbserver.h"
#include "db/dbfactory.h"
#include "io/ioserver.h"

namespace Db
{
__ImplementClass(Db::DbServer, 'DBSV', Core::RefCounted);
__ImplementSingleton(Db::DbServer);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
DbServer::DbServer():
    workDbInMemory(false)
{
    __ConstructSingleton;
    this->dbFactory = Db::Sqlite3Factory::Create();
}

//------------------------------------------------------------------------------
/**
*/
DbServer::~DbServer()
{
    n_assert(!this->IsStaticDatabaseOpen());
    n_assert(!this->IsGameDatabaseOpen());
    this->dbFactory = nullptr;
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
    Непосредственно откройте статическую базу данных. Статическая база данных содержит
    неизменяемые данные, доступные только для чтения.
*/
bool
DbServer::OpenStaticDatabase(const String& dbURI)
{
    n_assert(!this->IsStaticDatabaseOpen());
    n_assert(dbURI.IsValid());

    this->staticDatabase = DbFactory::Instance()->CreateDatabase();
    this->staticDatabase->SetURI(dbURI);
    // FIXME, почему это было эксклюзивом, оно все равно только для чтения
    this->staticDatabase->SetExclusiveMode(false);
    this->staticDatabase->SetAccessMode(Database::ReadOnly);
    this->staticDatabase->SetIgnoreUnknownColumns(true);
    this->staticDatabase->SetInMemoryDatabase(this->workDbInMemory);
    if (!this->staticDatabase->Open())
    {
        n_error("Не удалось открыть статическую базу данных «%s»!", dbURI.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Закройте статическую базу данных.
*/
void
DbServer::CloseStaticDatabase()
{
    n_assert(this->IsStaticDatabaseOpen());
    this->staticDatabase->Close();
    n_assert(this->staticDatabase->GetRefCount() == 1);
    this->staticDatabase = nullptr;
}

//------------------------------------------------------------------------------
/**
    Непосредственно откройте базу данных игры, содержащую динамические игровые данные.
*/
bool
DbServer::OpenGameDatabase(const String& dbURI)
{
    n_assert(!this->IsGameDatabaseOpen());
    n_assert(dbURI.IsValid());
    n_assert(!this->gameDatabase.isvalid());

    this->gameDatabase = DbFactory::Instance()->CreateDatabase();
    this->gameDatabase->SetURI(dbURI);
    this->gameDatabase->SetExclusiveMode(true);
    this->gameDatabase->SetAccessMode(Database::ReadWriteExisting);
    this->gameDatabase->SetIgnoreUnknownColumns(true);
    this->gameDatabase->SetInMemoryDatabase(this->workDbInMemory);
    if (!this->gameDatabase->Open())
    {
        n_error("Не удалось открыть базу данных игры «%s»!", dbURI.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Закройте базу данных игры.
*/
void
DbServer::CloseGameDatabase()
{
    n_assert(this->IsGameDatabaseOpen());
    this->gameDatabase->Close();
    n_assert(this->gameDatabase->GetRefCount() == 1);
    this->gameDatabase = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void
DbServer::Close()
{
    if (this->IsGameDatabaseOpen())
    {
        this->CloseGameDatabase();
    }
    if (this->IsStaticDatabaseOpen())
    {
        this->CloseStaticDatabase();
    }
}

//------------------------------------------------------------------------------
/**
    Это удалит текущую базу данных состояния игры.
*/
void
DbServer::DeleteCurrentGame(const String& profileURI)
{
    if (this->IsGameDatabaseOpen())
    {
        this->CloseGameDatabase();
    }

    // удалить текущую базу данных
    IO::IoServer* ioServer = IO::IoServer::Instance();
    if (ioServer->FileExists(profileURI))
    {
        bool dbDeleted = ioServer->DeleteFile(profileURI);
        n_assert(dbDeleted);
    }
}

//------------------------------------------------------------------------------
/**
    This opens the database in New Game mode: an original database will be
    copied into the user's profile directory into a Current Game State
    database and opened.
*/
bool
DbServer::OpenNewGame(const String& profileURI, const String& dbURI)
{
    n_assert(profileURI.IsValid());

    // убедитесь, что мы не открыты
    if (this->IsGameDatabaseOpen())
    {
        this->CloseGameDatabase();
    }
    
    if (!this->workDbInMemory)
    {        
        // убедитесь, что целевой каталог существует
        IO::IoServer* ioServer = IO::IoServer::Instance();
        ioServer->CreateDirectory(profileURI);
        
        // удалить текущую базу данных
        if (ioServer->FileExists(dbURI))
        {
            bool dbDeleted = ioServer->DeleteFile(dbURI);
            n_assert(dbDeleted);
        }
        IO::URI journalURI = dbURI + String("-journal");
        // удалить текущий файл журнала базы данных
        if (ioServer->FileExists(journalURI))
        {
            bool dbDeleted = ioServer->DeleteFile(journalURI);
            n_assert(dbDeleted);
        }
        bool dbCopied = ioServer->CopyFile("export:db/game.db4", dbURI);
        n_assert(dbCopied);
        // откройте скопированный файл базы данных
        return this->OpenGameDatabase(dbURI);       
    }
    else
    {
        return this->OpenGameDatabase("export:db/game.db4");
    }    
}

//------------------------------------------------------------------------------
/**
    Откроется база данных в режиме продолжения игры: текущая база данных игры.
    в каталоге профиля пользователя будет просто открыт.
*/
bool
DbServer::OpenContinueGame(const String& profileURI)
{
    n_assert(profileURI.IsValid());

    // убедитесь, что мы не открыты
    if (this->IsGameDatabaseOpen())
    {
        this->CloseGameDatabase();
    }
    
    // открыть текущую базу данных игры
    bool dbOpened = this->OpenGameDatabase(profileURI);
    return dbOpened;
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если текущая база данных игры существует.
*/
bool
DbServer::CurrentGameExists(const String& profileURI) const
{
    n_assert(profileURI.IsValid());
    return IO::IoServer::Instance()->FileExists(profileURI);
}

//------------------------------------------------------------------------------
/**
    Это откроет базу данных в режиме загрузки игры. Это перезапишет
    текущую базу данных игры с базой данных сохраненной игры и откройте ее как
    обычный.
*/
bool
DbServer::OpenLoadGame(const String& profileURI, const String& dbURI, const String& saveGameURI)
{
    n_assert(profileURI.IsValid());
    
    // убедитесь, что мы не открыты
    if (this->IsGameDatabaseOpen())
    {
        this->CloseGameDatabase();
    }
    
    IO::IoServer* ioServer = IO::IoServer::Instance();
    if (!this->workDbInMemory)
    {
        // убедитесь, что целевой каталог существует        
        ioServer->CreateDirectory(profileURI);
    }
    
    // проверьте, существует ли сохранение игры
    if (!ioServer->FileExists(saveGameURI))
    {
        return false;
    }
    
    if (!this->workDbInMemory)
    {
        // удалить текущую базу данных
        if (ioServer->FileExists(dbURI))
        {
            bool dbDeleted = ioServer->DeleteFile(dbURI);
            n_assert(dbDeleted);
        }
        bool dbCopied = ioServer->CopyFile(saveGameURI, dbURI);
        n_assert(dbCopied);
    }
    
    // откройте скопированный файл базы данных
    bool dbOpened = this->OpenGameDatabase(dbURI);
    return dbOpened;
}

//------------------------------------------------------------------------------
/**
    This creates a new save game by making a copy of the current world
    database into the savegame directory. If a savegame of that exists,
    it will be overwritten.
*/
bool
DbServer::CreateSaveGame(const String& profileURI, const String& dbURI, const String& saveGameURI)
{
    n_assert2(!this->workDbInMemory, "TODO: Сохранение игры из базы данных памяти еще не реализовано!");
    
    // убедитесь, что целевой каталог существует
    IO::IoServer* ioServer = IO::IoServer::Instance();
    ioServer->CreateDirectory(profileURI);

    // проверьте, существует ли текущая мировая база данных
    if (!ioServer->FileExists(dbURI))
    {
        return false;
    }

    // удалить файл сохранения игры, если он уже существует
    if (ioServer->FileExists(saveGameURI))
    {
        bool saveGameDeleted = ioServer->DeleteFile(saveGameURI);
        n_assert(saveGameDeleted);
    }
    else
    {
        String saveGamePath = saveGameURI.ExtractDirName();        
        ioServer->CreateDirectory(saveGamePath);
    }
    bool saveGameCopied = ioServer->CopyFile(dbURI, saveGameURI);
    n_assert(saveGameCopied);
    return true;
}

} // пространство имен БД
