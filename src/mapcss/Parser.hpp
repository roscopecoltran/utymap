#ifndef MAPCSS_PARSER_HPP_DEFINED
#define MAPCSS_PARSER_HPP_DEFINED

#include "Grammar.hpp"
#include "StyleSheet.hpp"

#include <memory>
#include <string>

namespace utymap { namespace mapcss {

template <typename Iterator>
class Parser
{
public:

    Parser() {}

    std::shared_ptr<StyleSheet> parse(Iterator begin, Iterator end)
    {
        error_.clear();
        auto style = std::shared_ptr<StyleSheet>(new StyleSheet());
        Grammar<Iterator> grammar;

        bool success = grammar.parse(begin, end, style);

        if (!success) {
            error_ = grammar.getLastError();
            return nullptr;
        }

        return style;
    }

    std::string getLastError() { return error_; }

private:
    std::string error_;
};

}} // ns end utymap::mapcss

#endif // MAPCSS_PARSER_HPP_DEFINED
