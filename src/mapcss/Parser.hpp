#ifndef MAPCSS_PARSER_HPP_DEFINED
#define MAPCSS_PARSER_HPP_DEFINED

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include "Stylesheet.hpp"

#include <string>

namespace utymap { namespace mapcss {

template <class T>
class Parser
{
public:

    Stylesheet* Parse(const T& data)
    {
        using boost::spirit::qi::double_;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::phrase_parse;
        using boost::spirit::ascii::space;
        using boost::phoenix::ref;

        double rN = 0.0;
        double iN = 0.0;
        bool r = phrase_parse(data.begin(), data.end(),

            //  Begin grammar
            (
            '(' >> double_[ref(rN) = _1]
            >> -(',' >> double_[ref(iN) = _1]) >> ')'
            | double_[ref(rN) = _1]
            ),
            //  End grammar

            space);

        // fail if we did not get a full match
        if (!r || data.begin() != data.end())
            return nullptr;

        return new Stylesheet();
    }

    std::string GetLastError() { return error_; }

private:
    std::string error_;
};

}} // ns end utymap::mapcss

#endif // MAPCSS_PARSER_HPP_DEFINED
