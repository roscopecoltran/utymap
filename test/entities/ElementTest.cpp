#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::entities;

class Counter: public ElementVisitor
{
public:
    int nodes;
    int ways;
    int relations;

    Counter(): nodes(0), ways(0), relations(0) {}

    void visitNode(const Node&) { nodes++; }
    void visitWay(const Way&) { ways++; }
    void visitRelation(const Relation&) { relations++; }
};

BOOST_AUTO_TEST_SUITE(Entities_Element)

BOOST_AUTO_TEST_CASE(GivenNode_Visit_IncrementsCounter)
{
    Counter counter;
    Node node;

    node.accept(counter);

    BOOST_CHECK_EQUAL(counter.nodes, 1);
    BOOST_CHECK_EQUAL(counter.ways, 0);
    BOOST_CHECK_EQUAL(counter.relations, 0);
}

BOOST_AUTO_TEST_CASE(GivenWay_Visit_IncrementsCounter)
{
    Counter counter;
    Way way;

    way.accept(counter);

    BOOST_CHECK_EQUAL(counter.nodes, 0);
    BOOST_CHECK_EQUAL(counter.ways, 1);
    BOOST_CHECK_EQUAL(counter.relations, 0);
}

BOOST_AUTO_TEST_CASE(GivenRelation_Visit_IncrementsCounter)
{
    Counter counter;
    Relation relation;

    relation.accept(counter);

    BOOST_CHECK_EQUAL(counter.nodes, 0);
    BOOST_CHECK_EQUAL(counter.ways, 0);
    BOOST_CHECK_EQUAL(counter.relations, 1);
}

BOOST_AUTO_TEST_SUITE_END()
