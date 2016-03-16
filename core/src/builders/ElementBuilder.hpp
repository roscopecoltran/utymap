#ifndef BUILDERS_ELEMENTBUILDER_HPP_DEFINED
#define BUILDERS_ELEMENTBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "entities/ElementVisitor.hpp"

namespace utymap { namespace builders {

// Provides the way to build specfic mesh from map data.
class ElementBuilder : public utymap::entities::ElementVisitor
{
public:
    ElementBuilder(const utymap::builders::BuilderContext& context) :
        context_(context)
    {
    }

    // Called when all objects in tile are processed.
    virtual void complete() = 0;

protected:
    const utymap::builders::BuilderContext& context_;
};

}}
#endif // BUILDERS_ELEMENTBUILDER_HPP_DEFINED
