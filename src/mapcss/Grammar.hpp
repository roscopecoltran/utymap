#ifndef MAPCSS_GRAMMAR_HPP_DEFINED
#define MAPCSS_GRAMMAR_HPP_DEFINED

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix.hpp>

#include "StyleSheet.hpp"

#include <vector>
#include <sstream>
#include <memory>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::standard;
namespace phoenix = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
    utymap::mapcss::SimpleSelector,
    (std::string, elementName_)
)

BOOST_FUSION_ADAPT_STRUCT(
    utymap::mapcss::Selector,
    (std::vector<utymap::mapcss::SimpleSelector>, simpleSelectors_)
)

namespace utymap { namespace mapcss {

template <typename Iterator>
struct CommentSkipper : public qi::grammar<Iterator>
{
    CommentSkipper() : CommentSkipper::base_type(skip, "PL/0")
    {
        skip = 
            ascii::space
            | ("/*" >> *(qi::char_ - "*/") >> "*/")
            | ("<!--" >> *(qi::char_ - "-->") >> "-->")
            ;
    }
    qi::rule<Iterator> skip;
};

template <typename Iterator>
class Grammar : qi::grammar<Iterator, CommentSkipper<Iterator> >
{
public:

    Grammar() :base_type(rulesetArray_, "MAPCSS") 
    {
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;
        using qi::blank;
        using qi::fail;
        using qi::lit;
        using qi::lexeme;
        using qi::on_error;
        using qi::raw;
        using qi::_val;
        using ascii::string;
        using ascii::char_;
        
        using phoenix::construct;
        using phoenix::val;

        rulesetArray_ = 
            +(ruleset_[phoenix::bind(&Grammar<Iterator>::pushRulesetArray, this)])
        ;

        ruleset_ = 
        (
            (selector_ % ',')[phoenix::bind(&Grammar<Iterator>::setCurrentSelectors, this, _1)]
            >  lit('{')
            >  (
                    -declaration_
                    > *(';' >> declaration_)
                    > -lit(';')
                )
            >  lit('}')
        );

        selector_ = 
            (+simpleSelector_[phoenix::bind(&Selector::addSimpleSelector, _val, _1)])
        ;

        declaration_ =
            lexeme[+(char_ - '}')]
            ;

        simpleSelector_ =
            elementName_[phoenix::bind(&SimpleSelector::setElementName, _val, _1)]
            > lit('[')
            > lexeme[+(char_ - ']')]
            > lit(']')
        ;

        elementName_ =
            string("node")
            | string("way")
            | string("area")
        ;

        rulesetArray_.name("stylesheet");
        ruleset_.name("ruleset");
        selector_.name("selector");
        declaration_.name("declaration");
        elementName_.name("elementName");

        on_error<fail>
        (
            rulesetArray_, 
            error_
            << val("Error! Expecting ")
            << _4                               // what failed?
            << val(" here: \"")
            << construct<std::string>(_3, _2)   // iterators to error-pos, end
            << val("\"")
            << std::endl
        );
    }

    bool parse(Iterator begin, Iterator end, std::shared_ptr<StyleSheet> s)
    {
        CommentSkipper<Iterator> skipper;
        s_ = s;

        bool success = phrase_parse(begin, end, rulesetArray_, skipper);

        // TODO error handling
        return success && begin == end;
    }

    void pushRulesetArray()
    {
        s_->rules_.insert(s_->rules_.end(), currentRules_.begin(), currentRules_.end());
        currentRules_.clear();
    }

    void setCurrentSelectors(const std::vector<Selector>& selectors)
    {
        for (const Selector& selector : selectors)
        {
            Rule rule;
            rule.selector_ = selector;
            currentRules_.push_back(rule);
        }
    }

    std::string getLastError() { return error_.str(); }

private:
    //std::string error_;
    std::stringstream error_;
    std::shared_ptr<StyleSheet> s_;
    std::vector<Rule> currentRules_;

    qi::rule<Iterator, CommentSkipper<Iterator> > ruleset_, rulesetArray_;
    qi::rule<Iterator, CommentSkipper<Iterator> > declaration_;
    qi::rule<Iterator, Selector(), CommentSkipper<Iterator> > selector_;
    qi::rule<Iterator, SimpleSelector()> simpleSelector_;
    qi::rule<Iterator, std::string()> elementName_;
};

}} // ns end utymap::mapcss

#endif // MAPCSS_GRAMMAR_HPP_DEFINED