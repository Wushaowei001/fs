
#include <problem.hxx>
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/handlers/base_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <utils/printers/vector.hxx>
#include <constraints/registry.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <gecode/driver.hh>
#include <constraints/gecode/utils/novelty_constraints.hxx>

namespace fs0 { namespace gecode {

BaseCSPHandler::~BaseCSPHandler() {
	if (_novelty) delete _novelty;
}
	
void BaseCSPHandler::init(const RPGData* bookkeeping) {
	_base_csp.init(MinHMaxValueSelector(&_translator, bookkeeping));
}

void BaseCSPHandler::registerTermVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerVariables(term, type, translator);
}

void BaseCSPHandler::registerFormulaVariables(const fs::AtomicFormula::cptr condition, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*condition);
	assert(component_translator);
	component_translator->registerVariables(condition, translator);
}

void BaseCSPHandler::registerTermConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerConstraints(term, type, translator);
}


void BaseCSPHandler::registerFormulaConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, translator);
}

//! A helper
template <typename T>
SimpleCSP::ptr instantiate(const SimpleCSP& csp,
						   const GecodeCSPVariableTranslator& translator,
						   const std::vector<ExtensionalConstraint>& extensional_constraints,
						   const T& layer) {
							   
	SimpleCSP* clone = static_cast<SimpleCSP::ptr>(csp.clone());
	translator.updateStateVariableDomains(*clone, layer);
	for (const ExtensionalConstraint& constraint:extensional_constraints) {
		if (!constraint.update(*clone, translator, layer)) {
			return nullptr;
		}
	}
	return clone;
}

SimpleCSP::ptr BaseCSPHandler::instantiate_csp(const GecodeRPGLayer& layer) const {
	SimpleCSP* csp = instantiate(_base_csp, _translator, _extensional_constraints, layer);
	if (!csp) return csp; // The CSP was detected unsatisfiable even before propagating anything
	
	// Post the novelty constraint
	if (_novelty) _novelty->post_constraint(*csp, layer);
	
	return csp;
}

SimpleCSP::ptr BaseCSPHandler::instantiate_csp(const State& state) const {
	return instantiate(_base_csp, _translator, _extensional_constraints, state);
}

void BaseCSPHandler::setup() {
	index();
}

void BaseCSPHandler::register_csp_variables() {
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		if (fs::FluentHeadedNestedTerm::cptr fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term)) {
			_translator.registerNestedTerm(fluent, CSPVariableType::Input);
			_extensional_constraints.push_back(ExtensionalConstraint(fluent, false));
			
		} else if (auto statevar = dynamic_cast<fs::StateVariable::cptr>(term)) {
			_translator.registerInputStateVariable(statevar->getValue());
		}
		
		else {
			registerTermVariables(term, CSPVariableType::Input, _translator);
		}
	}
	
	// We'll have one extensional constraint per predicate appearing on the condition / formula.
	for (const auto atom:_all_atoms) {
		_extensional_constraints.push_back(ExtensionalConstraint(atom, true));
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) registerFormulaVariables(condition, _translator);
}

void BaseCSPHandler::register_csp_constraints() {
// 	unsigned i = 0; _unused(i);
	
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		
		// These types of term do not require a custom translator
		if (dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term) ||
			dynamic_cast<fs::StateVariable::cptr>(term)) {
			continue;
		}
		
		registerTermConstraints(term, CSPVariableType::Input, _translator);
// 		FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *term << "): " << _translator); // Uncomment for extreme debugging
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) {
		registerFormulaConstraints(condition, _translator);
// 		FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *condition << "): " << _translator); // Uncomment for extreme debugging		
	}
	
	for (ExtensionalConstraint& constraint:_extensional_constraints) {
		constraint.register_constraints(_translator);
	}
}

std::ostream& BaseCSPHandler::print(std::ostream& os, const SimpleCSP& csp) const {
	return _translator.print(os, csp);
}

void BaseCSPHandler::createCSPVariables(bool use_novelty_constraint) {
	register_csp_variables();
	
	if (use_novelty_constraint) {
		create_novelty_constraint();
	}
	_translator.perform_registration();
}

void BaseCSPHandler::index_formula_elements(const std::vector<const fs::AtomicFormula*>& conditions, const std::vector<fs::Term::cptr>& terms) {
	const ProblemInfo& info = Problem::getInfo();
	std::unordered_set<const fs::Term*> inserted_terms;
	std::unordered_set<const fs::AtomicFormula*> inserted_conditions;
	
	for (auto condition:conditions) {
		if (auto relational = dynamic_cast<const fs::RelationalFormula*>(condition)) {
			
			if (relational->symbol() == fs::RelationalFormula::Symbol::EQ) {
				auto nested = dynamic_cast<const fs::NestedTerm*>(relational->lhs()); // TODO - CHECK SYMMETRICALLY FOR RHS()!!
				auto statevar = dynamic_cast<const fs::StateVariable*>(relational->lhs());
				
				if (nested && info.isPredicate(nested->getSymbolId())) {
					throw UnimplementedFeatureException("To implement");
				}
				
				if (statevar) {
					auto origin = statevar->getOrigin();
					if (info.isPredicate(origin->getSymbolId())) {
						// e.g. we had a condition clear(b) = 1, which we'll want to transform into an extensional constraint on the
						// CSP variable representing the constant 'b' wrt the extension given by the clear predicate
						
						auto value = dynamic_cast<const fs::IntConstant*>(relational->rhs());
						if (!value || value->getValue() != 1) {
							throw UnimplementedFeatureException("Only non-negated relational atoms are supported ATM");
						}
						
						// Mark the condition and term as inserted, so we do not need to insert it again!
						inserted_conditions.insert(condition);
						inserted_terms.insert(statevar);
						
						_all_atoms.insert(origin);
						
						// TODO - Perhaps StateVariable::all_terms should already return these terms?
						for (auto term:origin->getSubterms()) {
							auto tmp = term->all_terms();
							_all_terms.insert(tmp.cbegin(), tmp.cend()); // TODO These should go away once we deal with constants separately
						}
						
						_atom_state_variables.insert(Atom(statevar->getValue(), 1));
						continue;
					}
				}
			}
		}
	}
	
	std::copy_if(conditions.begin(), conditions.end(), std::inserter(_all_formulas, _all_formulas.begin()),
				 [&inserted_conditions] (const fs::AtomicFormula* atom) { return inserted_conditions.find(atom) == inserted_conditions.end(); });
	
	std::copy_if(terms.begin(), terms.end(), std::inserter(_all_terms, _all_terms.begin()),
				 [&inserted_terms] (const fs::Term* term) { return inserted_terms.find(term) == inserted_terms.end(); });
}

void BaseCSPHandler::extract_nested_term_support(const SimpleCSP* solution, std::vector<fs::FluentHeadedNestedTerm::cptr> nested_terms, const PartialAssignment& assignment, const Binding& binding, std::vector<Atom>& support) const {
	if (nested_terms.empty()) return;

	// And now of the derived state variables. Note that we keep track dynamically (with the 'insert' set) of the actual variables into which
	// the CSP solution resolves to prevent repetitions
	std::set<VariableIdx> inserted;

	for (fs::FluentHeadedNestedTerm::cptr fluent:nested_terms) {
		VariableIdx variable = fluent->interpretVariable(assignment, binding);
		if (inserted.find(variable) == inserted.end()) { // Don't push twice the support the same atom
			// ObjectIdx value = fluent->interpret(assignment, binding);
			ObjectIdx value = _translator.resolveValue(fluent, CSPVariableType::Input, *solution);
			support.push_back(Atom(variable, value));
			inserted.insert(variable);
		}
	}
}

} } // namespaces
