#ifndef FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED
#define FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED

#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/StringTable.hpp"
#include "utils/ElementUtils.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <array>

#include <iostream>

namespace utymap { namespace formats {

template<typename Visitor>
class OsmJsonParser
{
    const std::string IdAttributeName = "id";
    using ptree = boost::property_tree::ptree;
public:

    OsmJsonParser(utymap::index::StringTable& stringTable) :
        stringTable_(stringTable),
        idKey_(stringTable.getId(IdAttributeName))
    {
    }

    /// Parses osm json data from stream calling visitor.
    void parse(std::istream& istream, Visitor& visitor) const
    {
        static const std::array<std::string, 9> Features = { "buildings" };
            //{ "boundaries", "buildings", "earth", "landuse",  "places", "pois", "roads", "transit", "water" };

        ptree pt;
        read_json(istream, pt);
        for (const auto& feature : Features) {
            for (const ptree::value_type &f : pt.get_child(feature).get_child("features")) {
                const auto& type = f.second.get_child("geometry.type").data();
                if (type == "Polygon")
                    parsePolygon(visitor, f.second);
            }
        }
    }
private:
    /// Parses polygon.
    void parsePolygon(Visitor& visitor, const ptree &feature) const
    {
        utymap::entities::Relation relation;
        parseProperties(relation, feature.get_child("properties"));

        for (const ptree::value_type &geometry : feature.get_child("geometry.coordinates")) {
            auto area = std::make_shared<utymap::entities::Area>();
            if (!parseCoordinates(geometry.second, area->coordinates))
                throw std::invalid_argument("Invalid geometry.");
            
            area->id = 0;
            area->tags = relation.tags;

            relation.elements.push_back(area);
        }

        visitor.add(relation);
    }

    /// Parses coordinates list. If input data is invalid, return false.
    static bool parseCoordinates(const ptree &geometry, std::vector<utymap::GeoCoordinate>& coordinates)
    {
        for (const ptree::value_type &coordinate : geometry) {
            // skip invalid geometry
            if (std::distance(coordinate.second.begin(), coordinate.second.end()) != 2)
                return false;

            auto iter = coordinate.second.begin();
            double longitude = iter->second.get_value<double>();
            double latitude = (++iter)->second.get_value<double>();
            coordinates.push_back(utymap::GeoCoordinate(latitude, longitude));
        }

        // TODO check orientation
        std::reverse(coordinates.begin(), coordinates.end());

        return true;
    }

    void parseProperties(utymap::entities::Element& element, const ptree &properties) const
    {
        for (const ptree::value_type &property : properties) {
            std::uint32_t key = stringTable_.getId(property.first);
            if (key == idKey_)
                element.id = parseId(property.second.data());
            else
                element.tags.emplace_back(key, stringTable_.getId(property.second.data()));
        }

        std::sort(element.tags.begin(), element.tags.end());
    }

    std::uint64_t parseId(const std::string& value) const
    {
        // NOTE ID can be negative for relations: 
        // see http://wiki.openstreetmap.org/wiki/Osm2pgsql/schema#Processed_Data
        if (!value.empty()) {
            return utymap::utils::lexicalCast<std::uint64_t>(value[0] == '-'
                ? value.substr(1, value.size() - 1)
                : value);
        }
        return 0;
    }

const utymap::index::StringTable& stringTable_;
const std::uint32_t idKey_;
};

}}

#endif  // FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED