
#pragma once

#include <fs0_types.hxx>
#include <actions/action_schema.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

class ApplicableActionSet;
class BaseActionManager;

class GroundAction {
protected:
	//! The schema from which the action was grounded
	ActionSchema::cptr _schema;
	
	//! The indexes of the action binding, if any.
	const ObjectIdxVector _binding;
	
	//! The action preconditions  and effects
	const std::vector<AtomicFormula::cptr> _conditions;
	const std::vector<ActionEffect::cptr> _effects;
	
	//! The indexes of only those variables relevant to the applicability procedures of the action.
	const VariableIdxVector _scope;
	
	//! The indexes of all the state variables relevant to at least one of the effect or applicability procedures of the action.
	const VariableIdxVector _allRelevant;
	
	//! The indexes of all variables that might be affected by the action
	const VariableIdxVector _allAffected;
	
	//! Optionally, each action might have an associated constraint manager that handles its precondition constraints.
	//! Ownership of the pointer belongs to the action itself.
	// TODO - Ugly, refactor out of here
	BaseActionManager* _constraintManager;	
	
public:
	typedef const GroundAction* cptr;
	
	//! Trait required by aptk::DetStateModel
	typedef ActionIdx IdType;
	typedef ApplicableActionSet ApplicableSet;

	static const ActionIdx invalid_action_id;
	
	GroundAction(ActionSchema::cptr schema, const ObjectIdxVector& binding, const std::vector<AtomicFormula::cptr>& conditions, const std::vector<ActionEffect::cptr>& effects);
	~GroundAction();
	
	//! Returns the name of the action. To be implemented in each concrete action.
	const std::string& getName() const { return _schema->getName(); }
	
	const VariableIdxVector& getScope() const { return _scope; }
	const VariableIdxVector& getAllRelevantVariables() const { return _allRelevant; }
	const VariableIdxVector& getAffectedVariables() const { return _allAffected; }
	
	inline const std::vector<AtomicFormula::cptr>& getConditions() const { return _conditions; }
	
	inline const std::vector<ActionEffect::cptr>& getEffects() const { return _effects; }
	
	// TODO - Ugly, refactor out of here
	BaseActionManager* getManager() const { return _constraintManager; };
	void setManager(BaseActionManager* constraintManager) { _constraintManager = constraintManager; };
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GroundAction&  entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	//! Extract from the action conditions / effects the scope, relevant and affected variables.
	VariableIdxVector extractScope() const;
	VariableIdxVector extractRelevantVariables() const;
	VariableIdxVector extractAffectedVariables() const;
};


} // namespaces
