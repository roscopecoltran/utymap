#ifndef LSYS_LSYSTEM_HPP_DEFINED
#define LSYS_LSYSTEM_HPP_DEFINED

#include <map>
#include <memory>
#include <typeindex>
#include <vector>

namespace utymap { namespace lsys {

class Turtle;

class Rule
{
public:
    /// Returns word specified for this rule.
    //virtual const std::string& word() const = 0;

    /// Applies the rule to turtle.
    virtual void apply(Turtle& turtle) const = 0;

    virtual ~Rule() {}
};

/// Specifies lsystem created from lsys grammar.
struct LSystem final
{
    /// Defines type for list of rules.
    typedef std::vector<std::shared_ptr<Rule>> Rules;
    /// Defines production type with its probability.
    typedef std::vector<std::pair<double, Rules>> Productions;

    /// Amount of generations.
    int generations = 1;

    /// Default angle value.
    double angle = 1;

    /// Specifies axiom.
    Rules axiom;

    /// Specifies a map where key is word from V* alphabet and value is set of productions.
    std::map<std::type_index, Productions> productions;
};

}}

#endif // LSYS_LSYSTEM_HPP_DEFINED
