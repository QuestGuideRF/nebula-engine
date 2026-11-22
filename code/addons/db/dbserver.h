#ifndef DB_DBSERVER_H
#define DB_DBSERVER_H
#include "core/refcounted.h"
#include "core/singleton.h"
#include "db/database.h"
#include "db/sqlite3/sqlite3factory.h"
namespace Db
{
class DbServer : public Core::RefCounted
{
    __DeclareClass(DbServer);
    __DeclareSingleton(DbServer);
public:
    DbServer();
    virtual ~DbServer();
    bool OpenStaticDatabase(const Util::String& dbUri);
    bool OpenGameDatabase(const Util::String& dbUri);
    bool OpenNewGame(const Util::String& profileURI, const Util::String& dbURI);
    bool OpenContinueGame(const Util::String& profileURI);
    bool OpenLoadGame(const Util::String& profileURI, const Util::String& dbURI, const Util::String& saveGameURI);
    void DeleteCurrentGame(const Util::String& profileURI);
    void CloseStaticDatabase();
    void CloseGameDatabase();
    void Close();
    bool IsStaticDatabaseOpen() const;
    bool IsGameDatabaseOpen() const;
    bool CreateSaveGame(const Util::String& profileURI, const Util::String& dbURI, const Util::String& saveGameURI);
    const Ptr<Database>& GetGameDatabase() const;
    const Ptr<Database>& GetStaticDatabase() const;
    bool CurrentGameExists(const Util::String& profileURI) const;
    void SetWorkingDbInMemory(bool b);
private:
    bool workDbInMemory;
    Ptr<Db::Sqlite3Factory> dbFactory;
    Ptr<Database> staticDatabase;
    Ptr<Database> gameDatabase;
};
inline bool
DbServer::IsStaticDatabaseOpen() const
{
    return this->staticDatabase.isvalid();
}
inline bool
DbServer::IsGameDatabaseOpen() const
{
    return this->gameDatabase.isvalid();
}
inline const Ptr<Database>&
DbServer::GetStaticDatabase() const
{
    return this->staticDatabase;
}
inline const Ptr<Database>&
DbServer::GetGameDatabase() const
{
    return this->gameDatabase;
}
inline void
DbServer::SetWorkingDbInMemory(bool b)
{
    this->workDbInMemory = b;
}
}
#endif