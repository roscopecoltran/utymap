#ifndef UTILS_MAPCSSUTILS_HPP_DEFINED
#define UTILS_MAPCSSUTILS_HPP_DEFINED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "mapcss/Style.hpp"
#include "utils/GeoUtils.hpp"

#include <string>

namespace utymap { namespace utils {

    // Checks whether key exists.
    inline bool hasKey(const std::string& key,
                       utymap::index::StringTable& stringTable,
                       const utymap::mapcss::Style& style)
    {
        return style.has(stringTable.getId(key));
    }

    // Gets raw declaration value as string.
    inline std::shared_ptr<std::string> getString(const std::string& key,
                                                  utymap::index::StringTable& stringTable,
                                                  const utymap::mapcss::Style& style,
                                                  const std::string& defaultValue = "")
    {
        uint32_t keyId = stringTable.getId(key);
        return style.has(keyId)
            ? style.get(keyId)->value()
            : std::shared_ptr<std::string>(new std::string(defaultValue));
    }

    // Gets declaration value as double. Evaluates if necessary.
    inline double getDouble(const std::string& key,
                            const std::vector<utymap::entities::Tag>& tags,
                            utymap::index::StringTable& stringTable,
                            const utymap::mapcss::Style& style)
    {
        uint32_t keyId = stringTable.getId(key);
        return style.get(keyId)->evaluate(tags, stringTable);
    }

    // Gets declaration value as double without evaluation.
    inline double getDouble(const std::string& key,
                            utymap::index::StringTable& stringTable,
                            const utymap::mapcss::Style& style,
                            double defaultValue = 0)
    {
        uint32_t keyId = stringTable.getId(key);
        return style.has(keyId)
            ? std::stod(*style.get(keyId)->value())
            : defaultValue;
    }

    // Gets dimension value. Evaluation is not supported.
    inline double getDimension(uint32_t keyId,
                               utymap::index::StringTable& stringTable,
                               const utymap::mapcss::Style& style,
                               double size,
                               const utymap::GeoCoordinate& coordinate)
    {
        auto declaration = style.get(keyId);
        auto rawValue = declaration->value();

        char dimen = (*rawValue)[rawValue->size() - 1];
        double value = std::stod(rawValue->substr(0, rawValue->size() - 1));

        // defined in meters: use raw value
        if (dimen == 'm' && coordinate.latitude != 90)
            return coordinate.isValid() ? GeoUtils::getOffset(coordinate, value) : value;

        // relative to size
        if (dimen == '%')
            return size * value * 0.01;

        throw utymap::MapCssException("Unknown width dimension.");
    }

    // Gets dimension value. Evaluation is not supported.
    inline double getDimension(const std::string& key,
                               utymap::index::StringTable& stringTable,
                               const utymap::mapcss::Style& style,
                               double size,
                               double defaultValue,
                               const utymap::GeoCoordinate& coordinate = GeoCoordinate())
    {
        uint32_t keyId = stringTable.getId(key);
        if (!style.has(keyId))
            return defaultValue;

        return getDimension(keyId, stringTable, style, size, coordinate);
    }

    // Gets dimension value. Evaluation is not supported.
    inline double getDimension(const std::string& key,
                               utymap::index::StringTable& stringTable,
                               const utymap::mapcss::Style& style,
                               double size,
                               const utymap::GeoCoordinate& coordinate = GeoCoordinate())
    {
        return getDimension(stringTable.getId(key), stringTable, style, size, coordinate);
    }
}}

#endif // UTILS_MAPCSSUTILS_HPP_DEFINED
