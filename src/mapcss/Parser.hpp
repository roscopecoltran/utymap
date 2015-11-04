#ifndef MAPCSS_PARSER_HPP_DEFINED
#define MAPCSS_PARSER_HPP_DEFINED

#include "StyleSheet.hpp"
#include <boost/shared_ptr.hpp>

#include <string>

namespace utymap { namespace mapcss {

class Parser
{
public:

    boost::shared_ptr<StyleSheet> parse(const std::string& str);

    std::string getError();

private:
    template<typename Iterator>
    boost::shared_ptr<StyleSheet> parse(Iterator begin, Iterator end);

    std::string error_;
};

}}
#endif // MAPCSS_PARSER_HPP_DEFINED
