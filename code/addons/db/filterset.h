#pragma once
#ifndef DB_FILTERSET_H
#define DB_FILTERSET_H
#include "core/refcounted.h"
#include "attr/attribute.h"
#include "util/simpletree.h"
namespace Db
{
class Command;
class FilterSet : public Core::RefCounted
{
    __DeclareClass(FilterSet);
public:
    FilterSet();
    virtual ~FilterSet();
    void Clear();
    void ClearDirtyFlag();
    bool IsDirty() const;
    bool IsEmpty() const;
    void BeginBlock();
    void EndBlock();
    void AddEqualCheck(const Attr::Attribute& attr);
    void AddGreaterThenCheck(const Attr::Attribute& attr);
    void AddLessThenCheck(const Attr::Attribute& attr);
    void AddGreaterOrEqualCheck(const Attr::Attribute& attr);
    void AddLessOrEqualCheck(const Attr::Attribute& attr);
    void AddAnd();
    void AddOr();
    void AddNot();
    virtual Util::String AsSqlWhere() const;
    virtual void BindValuesToCommand(const Ptr<Command>& cmd, IndexT wildcardStartIndex);
protected:
    class Token
    {
    public:
        enum Type
        {
            Equal,
            NotEqual,
            Greater,
            Less,
            GreaterEqual,
            LessEqual,
            And,
            Or,
            Not,
            Block,
            Root,
        };
        Token();
        Token(Type t);
        Token(Type t, const Attr::Attribute& attr);
        Type GetType() const;
        const Attr::Attribute& GetAttr() const;
    private:
        Type type;
        Attr::Attribute attr;
    };
    Util::SimpleTree<Token> tokens;
    Util::SimpleTree<Token>::Node* curToken;
    Util::Array<Attr::Attribute> bindAttrs;
    bool isDirty;
};
inline void
FilterSet::ClearDirtyFlag()
{
    this->isDirty = false;
}
inline bool
FilterSet::IsDirty() const
{
    return this->isDirty;
}
inline FilterSet::Token::Token() :
    type(Root)
{
}
inline FilterSet::Token::Token(Type t) :
    type(t)
{
}
inline FilterSet::Token::Token(Type t, const Attr::Attribute& a) :
    type(t),
    attr(a)
{
}
inline bool
FilterSet::IsEmpty() const
{
    return (this->tokens.Root().Size() == 0);
}
inline FilterSet::Token::Type
FilterSet::Token::GetType() const
{
    return this->type;
}
inline const Attr::Attribute&
FilterSet::Token::GetAttr() const
{
    return this->attr;
}
}
#endif