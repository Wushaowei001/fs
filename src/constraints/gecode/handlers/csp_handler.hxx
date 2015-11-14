
#pragma once

#include <unordered_set>

#include <fs0_types.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/utils/variable_counter.hxx>
#include <constraints/gecode/utils/nested_fluent_element_translator.hxx>
#include <utils/config.hxx>


namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; class FluentHeadedNestedTerm; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { class GroundAction; class RPGData; }

namespace fs0 { namespace gecode {

class GecodeRPGLayer;
class GecodeCSPVariableTranslator;
class NoveltyConstraint;

//! The base interface class for all gecode CSP handlers
class GecodeCSPHandler {
public:
	typedef GecodeCSPHandler* ptr;
	typedef const GecodeCSPHandler* cptr;

	GecodeCSPHandler() : _base_csp(), _translator(_base_csp), _novelty(nullptr), _counter(Config::instance().useElementDontCareOptimization()) {}
	virtual ~GecodeCSPHandler();
	
	void init(const RPGData* bookkeeping);

	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	SimpleCSP::ptr instantiate_csp(const GecodeRPGLayer& layer) const;
	SimpleCSP::ptr instantiate_csp(const State& state) const;
	
	const GecodeCSPVariableTranslator& getTranslator() const { return _translator; }

	static void registerTermVariables(const fs::Term* term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerTermConstraints(const fs::Term* term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerFormulaVariables(const fs::AtomicFormula* condition, GecodeCSPVariableTranslator& translator);
	static void registerFormulaConstraints(const fs::AtomicFormula* condition, GecodeCSPVariableTranslator& translator);

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeCSPHandler& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const { return print(os, _base_csp); }
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;
	
protected:
	//! The base Gecode CSP
	SimpleCSP _base_csp;

	//! A translator to map planning variables with gecode variables
	GecodeCSPVariableTranslator _translator;
	
	NoveltyConstraint* _novelty;
	
	VariableCounter _counter;
	
	// All (distinct) FSTRIPS terms that participate in the CSP
	std::unordered_set<const fs::Term*> _all_terms;
	
	// All (distinct) FSTRIPS atomic formulas that participate in the CSP
	std::unordered_set<const fs::AtomicFormula*> _all_formulas;
	
	//! The set of nested fluent translators, one for each nested fluent in the set of terms modeled by this CSP
	std::vector<NestedFluentElementTranslator> _nested_fluent_translators;
	
	//! An index from the actual term to the position of the translator in the vector '_nested_fluent_translators'
	//! Note that we need to use the hash and equality specializations of the parent class Term pointer
	std::unordered_map<const fs::FluentHeadedNestedTerm*, unsigned, std::hash<const fs::Term*>, std::equal_to<const fs::Term*>> _nested_fluent_translators_idx;
	
	//! Return the nested fluent translator that corresponds to the given term
	const NestedFluentElementTranslator& getNestedFluentTranslator(const fs::FluentHeadedNestedTerm* fluent) const;
	
	virtual void create_novelty_constraint() = 0;
	
	virtual void index() = 0;

	void setup();
	
	//!
	void count_variables();
	
	void register_csp_variables();

	void register_csp_constraints();
	
	//! Registers the variables of the CSP into the CSP translator
	void createCSPVariables(bool use_novelty_constraint);
};


//! A CSP modeling and solving the progression between two RPG layers
class GecodeLayerCSPHandler : public GecodeCSPHandler { // TODO
public:
	typedef GecodeLayerCSPHandler* ptr;
	typedef const GecodeLayerCSPHandler* cptr;

protected:

};

} } // namespaces
