#ifndef MAPCSS_STYLEDECLARATION_HPP_INCLUDED
#define MAPCSS_STYLEDECLARATION_HPP_INCLUDED

#include "entities/Element.hpp"
#include "index/StringTable.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace utymap { namespace mapcss {

// Represents style declaration which support evaluation.
struct StyleDeclaration
{
    StyleDeclaration(std::uint32_t key, const std::string& value);

    ~StyleDeclaration();

    // Gets declaration key.
    std::uint32_t key() const;

    // Gets declaration value.
    std::shared_ptr<std::string> value() const;

    // Gets true if declaration should be evaluated
    bool isEval() const;

    // Evaluates double value from tags.
    double evaluate(const std::vector<utymap::entities::Tag>& tags, 
                    utymap::index::StringTable& stringTable) const;

private:
    struct StyleDeclarationImpl;
    std::unique_ptr<StyleDeclarationImpl> pimpl_;
};

}}
#endif  // MAPCSS_STYLEDECLARATION_HPP_INCLUDED
