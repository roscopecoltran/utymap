#ifndef _MAPCSS_PARSER_HPP_DEFINED_
#define _MAPCSS_PARSER_HPP_DEFINED_

namespace utymap { namespace mapcss {

class Parser
{
public:

    Parser(int value) : value(value) {}

    int getValue();

private:
    int value;
};

}} // ns end utymap::mapcss

#endif // _MAPCSS_PARSER_HPP_DEFINED_
