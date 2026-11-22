#ifndef DB2_SQLITE3FILTERSET_H
#define DB2_SQLITE3FILTERSET_H
#include "db/filterset.h"
namespace Db
{
class Sqlite3FilterSet : public FilterSet
{
    __DeclareClass(Sqlite3FilterSet);
public:
    virtual Util::String AsSqlWhere() const;
    virtual void BindValuesToCommand(const Ptr<Command>& cmd, IndexT wildcardStartIndex);
private:
    void RecurseBuildWhere(const Util::SimpleTree<Token>::Node* curNode, Util::String& inOutStr) const;
    static const Util::String OpenBracketFrag;
    static const Util::String CloseBracketFrag;
    static const Util::String AndFrag;
    static const Util::String OrFrag;
    static const Util::String NotFrag;
    static const Util::String DotFrag;
    static const Util::String EqualFrag;
    static const Util::String NotEqualFrag;
    static const Util::String GreaterFrag;
    static const Util::String LessFrag;
    static const Util::String GreaterEqualFrag;
    static const Util::String LessEqualFrag;
    static const Util::String WildcardFrag;
    static const Util::String TickFrag;
};
}
#endif