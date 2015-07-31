
#pragma once
#include <constraints/gecode/translators/base_translator.hxx>
#include <languages/fstrips/language.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {
	class GroundAction;
}

namespace fs0 { namespace language { namespace fstrips {
	class Term;
} } }

namespace fs0 { namespace gecode {

class GecodeFormulaTranslator : public GecodeBaseTranslator {
public:
	GecodeFormulaTranslator(const std::vector<fs::AtomicFormula::cptr>& conditions, const VariableIdxVector& relevant, SimpleCSP& csp, GecodeCSPTranslator& translator);

	//! The translator can optionally register any number of (probably temporary) CSP variables.
	void registerVariables(Gecode::IntVarArgs& variables);
	
	//! The translator can register any number of CSP constraints
	void registerConstraints() const;
	
protected:
	//! The conditions being translated
	const std::vector<fs::AtomicFormula::cptr>& _conditions;
	
	//! All the relevant variables
	const VariableIdxVector& _relevant;
	
	using GecodeBaseTranslator::registerConstraints;
	
};
} } // namespaces
