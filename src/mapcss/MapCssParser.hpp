#ifndef MAPCSS_MAPCSSPARSER_HPP_DEFINED
#define MAPCSS_MAPCSSPARSER_HPP_DEFINED

#include "mapcss/StyleSheet.hpp"

#include <istream>
#include <string>

namespace utymap { namespace mapcss {

// Provides the way to parse mapcss data into stylesheets object graph.
class MapCssParser
{
public:

    // Directory parameter specifies root path for import lookup.
    MapCssParser(const std::string& directory = "");

    // Parses stylesheet from string.
    StyleSheet parse(const std::string& content);

    // Parses stylsheet from input stream (e.g. file). 
    StyleSheet parse(std::istream& istream);

private:
    template<typename Iterator>
    StyleSheet parse(Iterator begin, Iterator end);

    std::string directory_;
};

}}
#endif // MAPCSS_MAPCSSPARSER_HPP_DEFINED
