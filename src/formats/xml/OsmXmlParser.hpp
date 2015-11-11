#ifndef FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED
#define FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED

#include "formats/Types.hpp"

#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstdint>

namespace utymap { namespace formats {

template<typename Visitor>
class OsmXmlParser
{
    using ptree = boost::property_tree::ptree;

public:

    OsmXmlParser() {}

    // Parses osm xml data from stream calling visitor
    void parse(std::istream& istream, Visitor& visitor)
    {
        ptree pt;
        read_xml(istream, pt);

        BOOST_FOREACH(ptree::value_type const& child, pt.get_child("osm"))
        {
            if (child.first == "node")
                parseNode(visitor, child);
            else if (child.first == "way")
                parseWays(visitor, child);
            else if (child.first == "relation")
                parseRelations(visitor, child);
            else if (child.first == "bounds")
                parseBounds(visitor, child);
        }
    }

private:

    void parseBounds(Visitor& visitor, ptree::value_type const& node)
    {
        const ptree & attributes = node.second.get_child("<xmlattr>");

        BoundingBox bbox;
        bbox.minLatitude = attributes.get_child("minlat").get_value<double>();
        bbox.minLongitude = attributes.get_child("minlon").get_value<double>();
        bbox.maxLatitude = attributes.get_child("maxlat").get_value<double>();
        bbox.maxLongitude = attributes.get_child("maxlon").get_value<double>();
        visitor.visitBounds(bbox);
    }

    void parseTag(ptree::value_type const& node, TagCollection& tags)
    {
        const ptree & attributes = node.second.get_child("<xmlattr>");

        Tag tag;
        tag.key = attributes.get_child("k").data();
        tag.value = attributes.get_child("v").data();
        tags.push_back(tag);
    }

    void parseNode(Visitor& visitor, ptree::value_type const& node)
    {
        const ptree & attributes = node.second.get_child("<xmlattr>");

        uint64_t id = attributes.get_child("id").get_value<uint64_t>();
        double lat = attributes.get_child("lat").get_value<double>();
        double lon = attributes.get_child("lon").get_value<double>();

        TagCollection tags;
        tags.reserve(2);
        BOOST_FOREACH(ptree::value_type const& child, node.second)
        {
            if (child.first == "tag")
                parseTag(child, tags);
        }

        visitor.visitNode(id, lat, lon, tags);
    }

    void parseWays(Visitor& visitor, ptree::value_type const& node)
    {
        uint64_t id = node.second.get_child("<xmlattr>")
            .get_child("id")
            .get_value<uint64_t>();

        std::vector<uint64_t> nodeIds;
        TagCollection tags;
        nodeIds.reserve(4);
        tags.reserve(2);
        BOOST_FOREACH(ptree::value_type const& child, node.second)
        {
            if (child.first == "nd")
            {
                uint64_t refId = child.second.get_child("<xmlattr>")
                    .get_child("ref")
                    .get_value<uint64_t>();
                nodeIds.push_back(refId);
            }
            else if (child.first == "tag")
                parseTag(child, tags);
        }

        visitor.visitWay(id, nodeIds, tags);
    }

    void parseRelations(Visitor& visitor, ptree::value_type const& node)
    {
        uint64_t id = node.second.get_child("<xmlattr>")
            .get_child("id")
            .get_value<uint64_t>();

        TagCollection tags;
        RelationMembers members;
        tags.reserve(2);
        members.reserve(2);
        BOOST_FOREACH(ptree::value_type const& child, node.second)
        {
            if (child.first == "member")
            {
                RelationMember member;
                const ptree & attributes = child.second.get_child("<xmlattr>");

                member.refId = attributes.get_child("ref").get_value<uint64_t>();
                member.type = attributes.get_child("type").get_value<std::string>();
                member.role = attributes.get_child("role").get_value<std::string>();
                members.push_back(member);
            }
            else if (child.first == "tag")
                parseTag(child, tags);
        }
        visitor.visitRelation(id, members, tags);
    }
};

}}

#endif  // FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED
