#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include <cstdint>
#include <string>
#include <vector>

namespace utymap { namespace mapcss {

struct Condition
{
    std::string key;
    std::string operation;
    std::string value;
};

struct Zoom
{
    std::uint8_t start;
    std::uint8_t end;
};

struct Selector
{
    std::string name;
    Zoom zoom;
    std::vector<Condition> conditions;
};

struct Declaration
{
    std::string key;
    std::string value;
};

struct Rule
{
    std::vector<Selector> selectors;
    std::vector<Declaration> declarations;
};

struct StyleSheet
{
    std::vector<Rule> rules;
};

}}
#endif // MAPCSS_STYLESHEET_HPP_DEFINED