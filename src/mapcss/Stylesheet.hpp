#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include <string>

namespace utymap { namespace mapcss {

struct SimpleSelector
{
public:

    void setElementName(const std::string& name) 
    {
        elementName_ = name;
    }

    std::string elementName_;
};

struct Selector
{
public:

    void addSimpleSelector(const SimpleSelector& selector)
    {
        simpleSelectors_.push_back(selector);
    }

    std::vector<SimpleSelector> simpleSelectors_;
};

struct Rule
{
public:

    Selector selector_;
};

struct StyleSheet
{
public:

    void addRule(const Rule& rule)
    {
        rules_.push_back(rule);
    }

    std::vector<Rule> rules_;
};

}} // ns end utymap::mapcss

#endif // MAPCSS_STYLESHEET_HPP_DEFINED
