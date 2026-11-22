#pragma once
#ifndef DB_SQLITE3TABLE_H
#define DB_SQLITE3TABLE_H
#include "db/table.h"
namespace Db
{
class Command;
class Sqlite3Table : public Table
{
    __DeclareClass(Sqlite3Table);
public:
    Sqlite3Table();
    virtual ~Sqlite3Table();
    virtual void Connect(const Ptr<Database>& db, ConnectMode connectMode, bool ignoreUnknownColumn);
    virtual void Disconnect(bool dropTable);
    virtual void CreateMultiColumnIndex(const Util::Array<Attr::AttrId>& columnIds);
    virtual void BindValueTable(const Ptr<ValueTable>& valueTable);
    virtual void UnbindValueTable();
    virtual void SetName(const Util::String& n);
    virtual void CommitChanges(bool resetModifiedState, bool useTransaction);
    virtual void CommitUncommittedColumns();
    virtual void CommitDeletedRows();
private:
    friend class Sqlite3Database;
    bool TableExists();
    void DropTable();
    void CreateTable();
    void ReadTableLayout(bool ignoreUnknownColumns);
    Util::String BuildColumnDef(const Column& column);
    void CompileInsertCommand();
    void CompileUpdateCommand();
    void CompileDeleteCommand();
    void ExecuteInsertCommand();
    void ExecuteUpdateCommand();
    void ExecuteDeleteCommand();
    void BindValueToCommand(const Ptr<Command>& cmd, IndexT wildcardIndex, IndexT valueTableColIndex, IndexT valueTableRowIndex);
    Ptr<Command> insertCommand;
    Ptr<Command> updateCommand;
    Ptr<Command> deleteCommand;
    static const Util::String InsertIntoFrag;
    static const Util::String OpenBracketFrag;
    static const Util::String CommaFrag;
    static const Util::String ValuesFrag;
    static const Util::String WildcardFrag;
    static const Util::String CloseBracketFrag;
    static const Util::String UpdateFrag;
    static const Util::String SetFrag;
    static const Util::String AssignWildcardFrag;
    static const Util::String TickFrag;
    static const Util::String IntegerFrag;
    static const Util::String RealFrag;
    static const Util::String TextFrag;
    static const Util::String BlobFrag;
    static const Util::String WhereFrag;
    static const Util::String DeleteFromFrag;
};
}
#endif