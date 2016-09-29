#include "Exceptions.hpp"
#include "mapcss/TextureAtlasParser.hpp"

#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/bind.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <streambuf>
#include <vector>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

using namespace utymap::mapcss;
using namespace utymap::meshing;

struct Region
{
    std::string groupName;
    int x;
    int y;
    int width;
    int height;
};

struct Atlas
{
    int width;
    int height;
    std::vector<Region> regions;
};

BOOST_FUSION_ADAPT_STRUCT(
    Region,
    (std::string, groupName)
    (int, x)
    (int, y)
    (int, width)
    (int, height)
)

BOOST_FUSION_ADAPT_STRUCT(
    Atlas,
    (int, width)
    (int, height)
    (std::vector<Region>, regions)
)

template <typename Iterator>
struct CommentSkipper : public qi::grammar<Iterator>
{
    CommentSkipper() : CommentSkipper::base_type(start, "comment")
    {
        start =
            ascii::space
            | ("/*" >> *(qi::char_ - "*/") >> "*/")
            ;
        start.name("comment");
    }
    qi::rule<Iterator> start;
};

template <typename Iterator>
struct RegionGrammar : qi::grammar < Iterator, Region(), CommentSkipper<Iterator> >
{
    RegionGrammar() : RegionGrammar::base_type(start, "region")
    {
        start =
            qi::lexeme[+(ascii::char_ - ',')] >> ',' >>
            qi::int_ >> ',' >>
            qi::int_ >> ',' >>
            qi::int_ >> ',' >>
            qi::int_
        ;

        start.name("region");
    }
    qi::rule<Iterator, Region(), CommentSkipper<Iterator> > start;
};

template <typename Iterator>
struct AtlasGrammar : qi::grammar < Iterator, Atlas(), CommentSkipper<Iterator>>
{
    AtlasGrammar() : AtlasGrammar::base_type(start, "atlas")
    {
        start =
            qi::int_ >> ',' >>
            qi::int_ >>
            +(region)
        ;

        qi::on_error<qi::fail>
            (
            start,
            error
            << phoenix::val("Error! Expecting ")
            << qi::_4
            << phoenix::val(" here: \"")
            << phoenix::construct<std::string>(qi::_3, qi::_2)
            << phoenix::val("\"")
            << std::endl
            );
    }

    RegionGrammar<Iterator> region;
    qi::rule<Iterator, Atlas(), CommentSkipper<Iterator>> start;
    std::stringstream error;
};

template<typename Iterator>
Atlas parse(Iterator begin, Iterator end)
{
    Atlas atlas;

    AtlasGrammar<Iterator> grammar;
    CommentSkipper<Iterator> skipper;

    if (!phrase_parse(begin, end, grammar, skipper, atlas))
        throw utymap::MapCssException(grammar.error.str());

    return atlas;
}

TextureAtlas TextureAtlasParser::parse(const std::string& content)
{
    if (content.empty())
        return TextureAtlas();

    Atlas atlas = ::parse(content.begin(), content.end());
    std::unordered_map<std::string, TextureGroup> groups;

    for (const auto& region: atlas.regions) {
        groups[region.groupName]
                .add(static_cast<std::uint16_t>(atlas.width),
                     static_cast<std::uint16_t>(atlas.height),
                     Rectangle(region.x, region.y, region.x + region.width, region.y + region.height));
    }

    return TextureAtlas(groups);
}

TextureAtlas TextureAtlasParser::parse(std::istream& istream)
{
    std::string atlasContent((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
    return parse(atlasContent);
}
