#include "mapcss/StyleSheet.hpp"
#include "mapcss/MapCssParser.hpp"
#include "Exceptions.hpp"

#include <boost/bind.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <streambuf>
#include <vector>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

using namespace utymap::mapcss;

BOOST_FUSION_ADAPT_STRUCT(
    Condition,
    (std::string, key)
    (std::string, operation)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    Zoom,
    (std::uint8_t, start)
    (std::uint8_t, end)
)

BOOST_FUSION_ADAPT_STRUCT(
    Selector,
    (std::string, name)
    (Zoom, zoom)
    (std::vector<Condition>, conditions)
)

BOOST_FUSION_ADAPT_STRUCT(
    Declaration,
    (std::string, key)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    Rule,
    (std::vector<Selector>, selectors)
    (std::vector<Declaration>, declarations)
)

BOOST_FUSION_ADAPT_STRUCT(
    StyleSheet,
    (std::vector<Rule>, rules)
)

template<typename Iterator>
void parse(const std::string& directory, Iterator begin, Iterator end, StyleSheet& stylesheet);

template <typename Iterator>
struct CommentSkipper : public qi::grammar<Iterator>
{
    CommentSkipper() : CommentSkipper::base_type(start, "comment")
    {
        start =
            ascii::space
            | ("/*" >> *(qi::char_ - "*/") >> "*/")
            | ("<!--" >> *(qi::char_ - "-->") >> "-->")
        ;
        start.name("comment");
    }
    qi::rule<Iterator> start;
};

template <typename Iterator>
struct ConditionGrammar : qi::grammar<Iterator, Condition(), CommentSkipper<Iterator> >
{
    ConditionGrammar() : ConditionGrammar::base_type(start, "condition")
    {
        start =
            '['
            > keyValue
            > -binary
            > -keyValue
            > ']'
        ;

        keyValue =
            qi::lexeme[+(ascii::char_ - (']' | binary))]
        ;

        binary =
              ascii::string("=")
            | ascii::string("!=")
            | ascii::string(">")
        ;
        start.name("condition");
        keyValue.name("keyValue");
        binary.name("binary");
    }
    qi::rule<Iterator, Condition(), CommentSkipper<Iterator> > start;
    qi::rule<Iterator, std::string()> keyValue;
    qi::rule<Iterator, std::string()> binary;
};

template <typename Iterator>
struct ZoomGrammar : qi::grammar < Iterator, Zoom(), CommentSkipper<Iterator> >
{
    ZoomGrammar() : ZoomGrammar::base_type(start, "zoom")
    {
        start =
            '|'
            > qi::lexeme['z' > qi::int_
            [
                boost::phoenix::at_c<0>(qi::_val) = qi::_1,
                boost::phoenix::at_c<1>(qi::_val) = qi::_1]
            ]
            > -qi::lit('-')
            > -qi::int_[boost::phoenix::at_c<1>(qi::_val) = qi::_1]
        ;
        start.name("zoom");
    }
    qi::rule<Iterator, Zoom(), CommentSkipper<Iterator> > start;
};


template <typename Iterator>
struct SelectorGrammar : qi::grammar<Iterator, Selector(), CommentSkipper<Iterator> >
{
    SelectorGrammar() : SelectorGrammar::base_type(start, "selector")
    {
        start =
            (
                (
                    ascii::string("node")
                  | ascii::string("way")
                  | ascii::string("area")
                  | ascii::string("relation")
                )
                > zoom
                > +condition
            )
            | ascii::string("canvas")
              > zoom
        ;
        start.name("selector");
    }
    qi::rule<Iterator, Selector(), CommentSkipper<Iterator> > start;
    ZoomGrammar<Iterator> zoom;
    ConditionGrammar<Iterator> condition;
};

template <typename Iterator>
struct DeclarationGrammar : qi::grammar < Iterator, Declaration(), CommentSkipper<Iterator> >
{
    DeclarationGrammar() : DeclarationGrammar::base_type(start, "declaration")
    {
        start =
              qi::lexeme[+(ascii::char_ - (qi::lit(':') | qi::lit('}')))]
            >> ':'
            > qi::lexeme[+(ascii::char_ - ';')]
            > ';'
        ;
        start.name("declaration");
    }
    qi::rule<Iterator, Declaration(), CommentSkipper<Iterator>> start;
};

template <typename Iterator>
struct RuleGrammar : qi::grammar < Iterator, Rule(), CommentSkipper<Iterator>>
{
    RuleGrammar() : RuleGrammar::base_type(start, "rule")
    {
        start =
            qi::eps
            >> selector % ','
            > '{'
            > +declaration
            > '}'
        ;
        start.name("rule");
    }
    qi::rule<Iterator, Rule(), CommentSkipper<Iterator>> start;
    SelectorGrammar<Iterator> selector;
    DeclarationGrammar<Iterator> declaration;
};

template <typename Iterator>
struct ImportGrammar : qi::grammar < Iterator, CommentSkipper<Iterator>>
{
    ImportGrammar(const std::string& directory, StyleSheet& stylesheet) : ImportGrammar::base_type(start, "import"),
        directory(directory), stylesheet(stylesheet)
    {
        start =
            ascii::string("@import url(\"") >
            qi::as_string[qi::lexeme[+(ascii::char_ - (qi::lit('"')))]]
              [boost::bind(&ImportGrammar::readImport, this, _1)]
            > "\");"
        ;
        start.name("import");
    }

private:

    void readImport(const std::string& url)
    {
        std::ifstream importFile(directory + url);
        std::string content((std::istreambuf_iterator<char>(importFile)), std::istreambuf_iterator<char>());
        // NOTE indirected recursion: caller must ensure that there is no recursive import.
        ::parse(directory, content.begin(), content.end(), stylesheet);
    }

    qi::rule<Iterator, CommentSkipper<Iterator>> start;
    const std::string directory;
    StyleSheet& stylesheet;
};

template <typename Iterator>
struct StyleSheetGrammar : qi::grammar < Iterator, StyleSheet(), CommentSkipper<Iterator>>
{
    // NOTE stylesheet is passed here only because of import grammar: I simply don't know
    // how to get stylesheet instance from attributes or context of import grammar. However, 
    // it seems to be possible somehow.
    StyleSheetGrammar(const std::string& directory, StyleSheet& stylesheet) : StyleSheetGrammar::base_type(start, "stylesheet"),
        import(directory, stylesheet)
    {
        start =
            qi::eps
            > *(rule | import)
        ;

        start.name("stylesheet");
        qi::on_error<qi::fail>
        (
            start,
            error
            << phoenix::val("Error! Expecting ")
            << qi::_4
            << phoenix::val(" here: \"")
            << phoenix::construct<std::string>(qi::_3, qi::_2)
            << phoenix::val("\"")
            << std::endl
        );
    }
    std::stringstream error;
    qi::rule<Iterator, StyleSheet(), CommentSkipper<Iterator>> start;
    RuleGrammar<Iterator> rule;
    ImportGrammar<Iterator> import;
};

MapCssParser::MapCssParser(const std::string& directory) : directory_(directory)
{
}

template<typename Iterator>
void parse(const std::string& directory, Iterator begin, Iterator end, StyleSheet& stylesheet)
{
    StyleSheetGrammar<Iterator> grammar(directory, stylesheet);
    CommentSkipper<Iterator> skipper;

    if (!phrase_parse(begin, end, grammar, skipper, stylesheet))
        throw utymap::MapCssException(grammar.error.str());
}

StyleSheet MapCssParser::parse(const std::string& str)
{
    StyleSheet stylesheet;
    ::parse(directory_, str.begin(), str.end(), stylesheet);

    // NOTE workaround for import: each import grammar adds empty rule
    auto& rules = stylesheet.rules;
    rules.erase(std::remove_if(rules.begin(), rules.end(), [](const Rule& rule) {
        return rule.selectors.empty();
    }), rules.end());

    rules.shrink_to_fit();
    return std::move(stylesheet);
}

StyleSheet MapCssParser::parse(std::istream& istream)
{
    // TODO from some reason, this approach will skip some spaces in declaration (e.g. gradient)
    //boost::spirit::istream_iterator begin(istream);
    //boost::spirit::istream_iterator end;
    //return parse(begin, end);
    std::string styleContent((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
    return parse(styleContent);
}
