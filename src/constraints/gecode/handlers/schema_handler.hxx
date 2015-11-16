
#pragma once

#include <constraints/gecode/handlers/base_action_handler.hxx>
#include <actions/action_schema.hxx>

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class ActionSchemaCSPHandler : public BaseActionCSPHandler {
public:
	typedef ActionSchemaCSPHandler* ptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const ActionSchema*>& schemata);

	ActionSchemaCSPHandler(const ActionSchema& action, bool approximate, bool use_novelty_constraint);
	ActionSchemaCSPHandler(const ActionSchema& action, const std::vector<fs::ActionEffect::cptr>& effects, bool approximate, bool use_novelty_constraint);
	virtual ~ActionSchemaCSPHandler() {}
protected:

	//! '_parameter_variables[i]' contains the index of the CSP variable that models the value of i-th parameter of the action schema
	std::vector<unsigned> _parameter_variables;
	
	//! An schema handler needs to index the action parameter CSP variables in addition
	//! to the other elements already indexed by the parent class
	void index_parameters();

	//! Return the action binding that corresponds to the given solution
	Binding build_binding_from_solution(SimpleCSP* solution) const;

	//! Return the (Lifted) ActionID corresponding to the given solution
	const ActionID* get_action_id(SimpleCSP* solution) const;
	
	//! Log some handler-related into
	virtual void log() const;
};

} } // namespaces