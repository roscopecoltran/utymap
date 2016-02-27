#ifndef BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/ElementBuilder.hpp"
#include "entities/Element.hpp"

#include <memory>

namespace utymap { namespace builders {

// Provides the way build terrain mesh.
class TerraBuilder : public utymap::builders::ElementBuilder
{
public:

    TerraBuilder(const utymap::builders::BuilderContext&);

    ~TerraBuilder();

    void visitNode(const utymap::entities::Node&);

    void visitWay(const utymap::entities::Way&);

    void visitArea(const utymap::entities::Area&);

    void visitRelation(const utymap::entities::Relation&);

    void complete();

private:
    class TerraBuilderImpl;
    std::unique_ptr<TerraBuilderImpl> pimpl_;
};

}}

#endif // BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED
