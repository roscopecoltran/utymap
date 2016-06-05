#ifndef MAPCSS_STYLE_HPP_INCLUDED
#define MAPCSS_STYLE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "mapcss/StyleDeclaration.hpp"
#include "index/StringTable.hpp"
#include "utils/CompatibilityUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

namespace utymap { namespace mapcss {

// Represents style for element.
struct Style
{
    typedef std::uint32_t key_type;
    typedef std::shared_ptr<utymap::mapcss::StyleDeclaration> value_type;

    std::unordered_map<key_type, value_type> declarations;

    Style(utymap::index::StringTable& stringTable) 
        : stringTable_(stringTable),
          declarations()
    {
    }

    inline bool has(key_type key) const
    {
        return declarations.find(key) != declarations.end();
    }

    inline bool has(key_type key, const std::string& value) const
    {
        auto it = declarations.find(key);
        return it != declarations.end() && *it->second->value() == value;
    }

    inline void put(const value_type& declaration)
    {
        declarations[declaration->key()] = declaration;
    }

    inline value_type get(key_type key) const
    {
        auto it = declarations.find(key);
        if (it == declarations.end())
            throw MapCssException(std::string("Cannot find declaration with the key: ") + stringTable_.getString(key));

        return it->second;
    }

    // Has key.
    inline bool hasKey(const std::string& key)
    {
        return has(stringTable_.getId(key));
    }
    
    // Gets string by given key.
    inline std::shared_ptr<std::string> getString(const std::string& key,
                                                  const std::string& defaultValue = "") const
    {
        key_type keyId = stringTable_.getId(key);
        return has(keyId)
            ? get(keyId)->value()
            : std::make_shared<std::string>(defaultValue);
    }

    // Gets double value. Evaluate if necessary.
    inline double getValue(const std::string& key,
                           const std::vector<utymap::entities::Tag>& tags) const
    {
        key_type keyId = stringTable_.getId(key);

        auto declaration = get(keyId);

        return declaration->isEval()
                ? declaration->evaluate<double>(tags, stringTable_)
                : getValue(declaration);
    }

    // Gets double value. Evaluate is not supported.
    inline double getValue(const std::string& key, 
                           double size, 
                           double defaultValue = 0) const
    {
        return getValue(key, size, GeoCoordinate(), defaultValue);
    }

    // Gets double value. Evaluate is not supported.
    inline double getValue(const std::string& key,
                           const utymap::GeoCoordinate& coordinate,
                           double defaultValue = 0) const
    {
        return getValue(key, 1, coordinate, defaultValue);
    }

    // Gets double value. Evaluate is not supported.
    inline double getValue(const std::string& key,
                           double size,
                           const utymap::GeoCoordinate& coordinate,
                           double defaultValue = 0) const
    {
        key_type keyId = stringTable_.getId(key);

        return getValue(keyId, size, coordinate, defaultValue);
    }

    // Gets double value. Evaluate is not supported.
    inline double getValue(key_type keyId,
                           double size,
                           const utymap::GeoCoordinate& coordinate,
                           double defaultValue = 0) const
    {
        if (!has(keyId))
            return defaultValue;

        auto declaration = get(keyId);
        if (declaration->isEval())
            throw MapCssException(std::string("Cannot get value: it should be evaluated! Key: ") + stringTable_.getString(keyId));

        return getValue(declaration, size, coordinate);
    }

private:

    inline double getValue(const value_type& declaration,
                           double size = 1,
                           const utymap::GeoCoordinate& coordinate = GeoCoordinate()) const
    {
        auto rawValue = declaration->value();
        char dimen = (*rawValue)[rawValue->size() - 1];

        // in meters
        if (dimen == 'm' && coordinate.isValid()) {
            double value = std::stod(rawValue->substr(0, rawValue->size() - 1));
            return utymap::utils::GeoUtils::getOffset(coordinate, value);
        }

        // relative to size
        if (dimen == '%') {
            double value = std::stod(rawValue->substr(0, rawValue->size() - 1));
            return size * value * 0.01;
        }

        return std::stod(*rawValue);
    }

    utymap::index::StringTable& stringTable_;
};

}}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
