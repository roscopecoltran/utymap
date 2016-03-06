#include "entities/Element.hpp"
#include "mapcss/StyleDeclaration.hpp"
#include "utils/CompatibilityUtils.hpp"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace utymap::entities;
using namespace utymap::mapcss;

namespace {

    struct Nil {};
    struct Signed;  
    struct Tree;

    typedef boost::variant<
        Nil,
        double,
        std::string,
        boost::recursive_wrapper<Signed>,
        boost::recursive_wrapper<Tree>
    >
    Operand;

    struct Signed
    {
        char sign;
        Operand operand;
    };

    struct Operation
    {
        char operator_;
        Operand operand;
    };

    struct Tree
    {
        Operand first;
        std::list<Operation> rest;
    };

    // Evaluates AST.
    struct Evaluator
    {
        typedef double result_type;

        const std::vector<Tag>& tags;

        Evaluator(const std::vector<Tag>& tags) : tags(tags)
        {
        }

        double operator()(Nil) const { return 0; }
        double operator()(double n) const { return n; }

        double operator()(const Operation& o, double lhs) const
        {
            double rhs = boost::apply_visitor(*this, o.operand);
            switch (o.operator_)
            {
                case '+': return lhs + rhs;
                case '-': return lhs - rhs;
                case '*': return lhs * rhs;
                case '/': return lhs / rhs;
            }
            return 0;
        }

        double operator()(const std::string& tagKey) const
        {
            return 0;
        }

        double operator()(const Signed& s) const
        {
            double rhs = boost::apply_visitor(*this, s.operand);
            switch (s.sign)
            {
                case '-': return -rhs;
                case '+': return +rhs;
            }
            return 0;
        }

        double operator()(const Tree& tree) const
        {
            double state = boost::apply_visitor(*this, tree.first);
            for (const Operation& oper : tree.rest)
                state = (*this)(oper, state);
            return state;
        }
    };

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    // Grammar for parsing string into AST.
    template <typename Iterator>
    struct EvalGrammar : qi::grammar<Iterator, Tree(), ascii::space_type>
    {
        EvalGrammar() : EvalGrammar::base_type(expression)
        {
            qi::double_type double_;
            qi::char_type char_;

            expression =
                "eval(" >> term >> *((char_("+") >> term) | (char_("-") >> term)) >> ")"
                ;

            term =
                factor >> *((char_("*") >> factor) | (char_("/") >> factor))
                ;

            tag =
                "tag(\"" >> qi::lexeme[+(ascii::char_ - '"')] >> "\")"
                ;

            factor =
                double_
                | tag
                | '(' >> expression >> ')'
                | (char_("-") >> factor)
                | (char_("+") >> factor)
                ;
        }

        qi::rule<Iterator, std::string()> tag;
        qi::rule<Iterator, Tree(), ascii::space_type> expression;
        qi::rule<Iterator, Tree(), ascii::space_type> term;
        qi::rule<Iterator, Operand(), ascii::space_type> factor;
    };
}

BOOST_FUSION_ADAPT_STRUCT(
    Signed,
    (char, sign)
    (Operand, operand)
)

BOOST_FUSION_ADAPT_STRUCT(
    Operation,
    (char, operator_)
    (Operand, operand)
)

BOOST_FUSION_ADAPT_STRUCT(
    Tree,
    (Operand, first)
    (std::list<Operation>, rest)
)

struct StyleDeclaration::StyleDeclarationImpl
{
    StyleDeclarationImpl(std::uint64_t key, const std::string& value) :
        key_(key), value_(value), tree_(new Tree())
    {
        EvalGrammar<std::string::const_iterator> grammar;

        auto iter = value_.begin();
        auto end = value_.end();
        boost::spirit::ascii::space_type space;
        bool r = phrase_parse(iter, end, grammar, space, *tree_);

        if (!r || iter != end)
            tree_.reset();
    }

    double evaluate(const std::vector<Tag>& tags)
    {
        if (tree_ == nullptr)
            return std::stod(value_);

        return Evaluator(tags)(*tree_);
    }

private:
    const std::uint64_t key_;
    const std::string value_;
    std::shared_ptr<Tree> tree_;
};

StyleDeclaration::StyleDeclaration(std::uint64_t key, const std::string& value) :
    pimpl_(new StyleDeclarationImpl(key, value))
{
}

StyleDeclaration::~StyleDeclaration()
{
}

double StyleDeclaration::evaluate(const std::vector<Tag>& tags)
{
    return pimpl_->evaluate(tags);
}
