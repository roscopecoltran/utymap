#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include <cstdint>
#include <string>
#include <vector>

namespace utymap { namespace mapcss {

// Represents condition.
struct Condition
{
    std::string key;
    std::string operation;
    std::string value;
};

// Represents map zoom level.
struct Zoom
{
    std::uint8_t start;
    std::uint8_t end;
};

// Represents selector.
struct Selector
{
    std::string name;
    Zoom zoom;
    std::vector<Condition> conditions;
};

// Represents declaration.
struct Declaration
{
    std::string key;
    std::string value;
};

// Represents rule.
struct Rule
{
    std::vector<Selector> selectors;
    std::vector<Declaration> declarations;
};

// Represents stylesheet.
struct StyleSheet
{
    std::vector<Rule> rules;
};

std::ostream& operator<<(std::ostream &stream, const Condition &c);
std::ostream& operator<<(std::ostream &stream, const Zoom &z);
std::ostream& operator<<(std::ostream &stream, const Selector &s);
std::ostream& operator<<(std::ostream &stream, const Declaration &d);
std::ostream& operator<<(std::ostream &stream, const Rule &r);
std::ostream& operator<<(std::ostream &stream, const StyleSheet &s);

}}
#endif // MAPCSS_STYLESHEET_HPP_DEFINED
