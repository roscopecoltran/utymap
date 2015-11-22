#ifndef ENTITIES_ELEMENTVISITOR_HPP_DEFINED
#define ENTITIES_ELEMENTVISITOR_HPP_DEFINED

namespace utymap { namespace entities {

class Node;
class Way;
class Relation;

class ElementVisitor
{
public:
    virtual void visitNode(const Node&) = 0;
    virtual void visitWay(const Way&) = 0;
    virtual void visitRelation(const Relation&) = 0;
};

}}

#endif // ENTITIES_ELEMENTVISITOR_HPP_DEFINED
