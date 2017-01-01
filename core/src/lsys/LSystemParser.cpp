#include "lsys/LSystemParser.hpp"

#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/bind.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace utymap::lsys;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

typedef std::map<LSystem::RuleType, LSystem::Productions, RuleComparator> ProductionMap;

BOOST_FUSION_ADAPT_STRUCT(
    LSystem,
    (int, generations)
    (double, angle)
    (double, width)
    (double, length)
    (LSystem::Rules, axiom)
    (ProductionMap, productions)
)

namespace {

const std::shared_ptr<MoveForwardRule> forward = std::make_shared<MoveForwardRule>();
const std::shared_ptr<JumpForwardRule> jump = std::make_shared<JumpForwardRule>();

struct RuleTable : qi::symbols<char, LSystem::RuleType>
{
    RuleTable()
    {
        add
            ("F", forward)
            ("f", jump)
        ;
    }
};

struct WordRuleFactory
{
    template <typename Arg>
    struct result { typedef LSystem::RuleType type; };

    template<typename Arg>
    LSystem::RuleType operator()(const Arg& c) const
    {
        return std::make_shared<WordRule>(std::string(1, c));
    }
};

struct ProductionInserter
{
    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    struct result { typedef void type; };

    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    void operator()(Arg1& dict, Arg2& predcessor, Arg3& probability, Arg4& successor) const
    {
        dict[predcessor].push_back(std::make_pair(probability, successor));
    }
};

template <typename Iterator>
struct CommentSkipper : qi::grammar<Iterator>
{
    CommentSkipper() : CommentSkipper::base_type(start, "comment")
    {
        start =
            qi::lit(' ') |
            ('#' >> *(qi::char_ - '\n') >> '\n')
        ;
        start.name("comment");
    }
    qi::rule<Iterator> start;
};

template <typename Iterator>
struct RuleGrammar : qi::grammar <Iterator, LSystem::RuleType(), CommentSkipper<Iterator>>
{
    RuleGrammar() : RuleGrammar::base_type(start, "rule")
    {
        word =
            qi::lexeme[ascii::char_ - (qi::lit(' ') | '\n')][qi::_val = wordRuleFactory(qi::_1)]
        ;

        start = ruleTable | word;

        word.name("word");
        start.name("rule");
    }

    RuleTable ruleTable;
    boost::phoenix::function<WordRuleFactory> wordRuleFactory;

    qi::rule<Iterator, LSystem::RuleType(), CommentSkipper<Iterator>> start;
    qi::rule<Iterator, LSystem::RuleType(), CommentSkipper<Iterator>> word;
};

template <typename Iterator>
struct ProductionGrammar : qi::grammar <Iterator, ProductionMap(), CommentSkipper<Iterator>>
{
    ProductionGrammar() : ProductionGrammar::base_type(start, "production")
    {
        probability = 
                ('(' > qi::double_ > ')') |
                qi::attr(1)
        ;

        start =
            (rule >
            probability >
            "->" >
            +rule)[inserter(qi::_val, qi::_1, qi::_2, qi::_3)] % '\n'
        ;
    }

    boost::phoenix::function< ProductionInserter> inserter;
    RuleGrammar<Iterator> rule;
    qi::rule<Iterator, double(), CommentSkipper<Iterator>> probability;
    qi::rule<Iterator, ProductionMap(), CommentSkipper<Iterator>> start;
};

template <typename Iterator>
struct LSystemGrammar : qi::grammar <Iterator, LSystem(), CommentSkipper<Iterator>>
{
    LSystemGrammar() : LSystemGrammar::base_type(start, "lsystem")
    {
        start =
            (qi::lit("generations:") > qi::int_) >
            (qi::lit("angle:") > qi::double_) >
            (qi::lit("width:") > qi::double_) >
            (qi::lit("length:") > qi::double_) >
            (qi::lit("axiom:") > +rule > '\n') >
            production
        ;

        start.name("lsystem");
        rule.name("production");
        production.name("production");

        qi::on_error<qi::fail>
        (
            start,
            error
            << phx::val("Error! Expecting ")
            << qi::_4
            << phx::val(" here: \"")
            << phx::construct<std::string>(qi::_3, qi::_2)
            << phx::val("\"")
            << std::endl
        );
    }
    std::stringstream error;

    RuleGrammar<Iterator> rule;
    ProductionGrammar<Iterator> production;
    qi::rule<Iterator, LSystem(), CommentSkipper<Iterator>> start;
};

template<typename Iterator>
void parse(Iterator begin, Iterator end, LSystem& lsystem)
{
    LSystemGrammar<Iterator> grammar;
    CommentSkipper<Iterator> skipper;

    if (!phrase_parse(begin, end, grammar, skipper, lsystem))
        throw std::domain_error(std::string("Cannot parse lsystem:") + grammar.error.str());
}
}

LSystem LSystemParser::parse(const std::string& str) const
{
    LSystem lsystem;
    ::parse(str.begin(), str.end(), lsystem);
    return lsystem;
}

LSystem LSystemParser::parse(std::istream& istream) const
{
    std::string content((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
    return parse(content);
}
