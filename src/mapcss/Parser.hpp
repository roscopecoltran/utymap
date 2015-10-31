#ifndef MAPCSS_PARSER_HPP_DEFINED
#define MAPCSS_PARSER_HPP_DEFINED

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

#endif // MAPCSS_PARSER_HPP_DEFINED
