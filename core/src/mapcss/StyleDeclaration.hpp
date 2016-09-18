#ifndef MAPCSS_STYLEDECLARATION_HPP_INCLUDED
#define MAPCSS_STYLEDECLARATION_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleEvaluator.hpp"
#include "utils/ElementUtils.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace utymap { namespace mapcss {

// Represents style declaration which support evaluation.
struct StyleDeclaration final
{
    StyleDeclaration(std::uint32_t key, const std::string& value) :
        key_(key),
        value_(std::make_shared<std::string>(value)),
        tree_(StyleEvaluator::parse(value))
    {
    }

    ~StyleDeclaration() {}

    // Gets declaration key.
    std::uint32_t key() const { return key_; };

    // Gets declaration value.
    std::shared_ptr<std::string> value() const { return value_; };

    // Gets true if declaration should be evaluated
    bool isEval() const { return tree_ != nullptr; }

    // Evaluates expression using tags
    template <typename T>
    T evaluate(const std::vector<utymap::entities::Tag>& tags,
               utymap::index::StringTable& stringTable) const
    {
        if (!isEval())
            throw utymap::MapCssException("Cannot evaluate raw value.");

        return StyleEvaluator::evaluate<T>(*tree_, tags, stringTable);
    }

private:

    const std::uint32_t key_;
    std::shared_ptr<std::string> value_;
    std::shared_ptr<StyleEvaluator::Tree> tree_;
};

}}
#endif  // MAPCSS_STYLEDECLARATION_HPP_INCLUDED
