#ifndef MAPCSS_PARSER_HPP_DEFINED
#define MAPCSS_PARSER_HPP_DEFINED

#include "StyleSheet.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <string>

namespace utymap { namespace mapcss {

// Provides the way to parse mapcss data into stylesheets object graph.
class Parser
{
public:

    // Parses stylesheet from string.
    boost::shared_ptr<StyleSheet> parse(const std::string& str);

    // Parses stylsheet from input stream (e.g. file).
    boost::shared_ptr<StyleSheet> parse(boost::spirit::istream_iterator begin, 
                                        boost::spirit::istream_iterator end);
    
    // Returns last error if parse fails.
    std::string getError() { return error_;  };

private:
    template<typename Iterator>
    boost::shared_ptr<StyleSheet> parse(Iterator begin, Iterator end);

    std::string error_;
};

}}
#endif // MAPCSS_PARSER_HPP_DEFINED
