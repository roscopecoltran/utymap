#ifndef MAPCSS_STYLE_HPP_INCLUDED
#define MAPCSS_STYLE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "mapcss/StyleDeclaration.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace utymap { namespace mapcss {

// Represents style for element.
struct Style final
{    
    std::unordered_map<std::uint32_t, const StyleDeclaration&> declarations;

    Style(const std::vector<utymap::entities::Tag>& tags,
          utymap::index::StringTable& stringTable) :
        declarations(), stringTable_(stringTable), tags_(tags)
    {
    }

    Style(Style&& other) : stringTable_(other.stringTable_)
    {
        declarations = std::move(other.declarations);
        tags_ = std::move(other.tags_);
    }

    Style(const Style &) = default;

    Style& operator=(const Style&) = delete;
    Style& operator=(Style&&) = delete;

    bool has(std::uint32_t key) const
    {
        return declarations.find(key) != declarations.end();
    }

    bool has(std::uint32_t key, const std::string& value) const
    {
        auto it = declarations.find(key);
        return it != declarations.end() && it->second.value() == value;
    }

    void put(const StyleDeclaration& declaration)
    {
        declarations.emplace(declaration.key(), declaration);
    }

    const StyleDeclaration& get(std::uint32_t key) const
    {
        auto it = declarations.find(key);
        if (it == declarations.end())
            throw MapCssException(std::string("Cannot find declaration with the key: ") + stringTable_.getString(key));

        return it->second;
    }

    // Gets string by given key. Empty string by default
    std::string getString(const std::string& key) const
    {
        std::uint32_t keyId = stringTable_.getId(key);
        return getString(keyId);
    }
    
    // Gets string by given key. Empty string by default
    std::string getString(std::uint32_t keyId) const
    {
        if (!has(keyId))
            return "";

        auto& declaration = get(keyId);

        return declaration.isEval()
               ? declaration.evaluate<std::string>(tags_, stringTable_)
               : declaration.value();
    }

    // Gets double value or zero.
    double getValue(const std::string& key,
                    double size = 1,
                    const utymap::GeoCoordinate& coordinate = GeoCoordinate()) const
    {
        std::uint32_t keyId = stringTable_.getId(key);
        return getValue(keyId, size, coordinate);
    }

    // Gets double value or zero.
    double getValue(std::uint32_t keyId,
                    double size = 1,
                    const utymap::GeoCoordinate& coordinate = GeoCoordinate()) const
    {
        if (!has(keyId))
            return 0;

        const auto& declaration = get(keyId);
        const auto& rawValue = declaration.value();
        char dimen = rawValue[rawValue.size() - 1];

        if (dimen == 'm') {
            double value = utymap::utils::parseDouble(rawValue.substr(0, rawValue.size() - 1));
            return coordinate.isValid()
                ? utymap::utils::GeoUtils::getOffset(coordinate, value)
                : value;
        }

        // relative to size
        if (dimen == '%') {
            double value = utymap::utils::parseDouble(rawValue.substr(0, rawValue.size() - 1));
            return size * value * 0.01;
        }

        return declaration.isEval()
                ? declaration.evaluate<double>(tags_, stringTable_)
                : utymap::utils::parseDouble(rawValue);
    }

private:
    utymap::index::StringTable& stringTable_;
    std::vector<utymap::entities::Tag> tags_;
};

}}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
