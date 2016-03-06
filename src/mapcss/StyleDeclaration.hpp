#ifndef MAPCSS_STYLEDECLARATION_HPP_INCLUDED
#define MAPCSS_STYLEDECLARATION_HPP_INCLUDED

#include "entities/Element.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace utymap { namespace mapcss {

// Represents style declaration which support evaluation.
struct StyleDeclaration
{
    StyleDeclaration(std::uint64_t key, const std::string& value);
    ~StyleDeclaration();

    double evaluate(const std::vector<utymap::entities::Tag>& tags);

private:
    struct StyleDeclarationImpl;
    std::unique_ptr<StyleDeclarationImpl> pimpl_;
};

}}
#endif  // MAPCSS_STYLEDECLARATION_HPP_INCLUDED
