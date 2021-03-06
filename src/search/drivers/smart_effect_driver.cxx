
#include <search/drivers/smart_effect_driver.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <constraints/gecode/handlers/lifted_effect_csp.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {

std::unique_ptr<FS0SearchAlgorithm>
SmartEffectDriver::create(const Config& config, const GroundStateModel& model) const {
	LPT_INFO("main", "Using the smart-effect driver");
	const Problem& problem = model.getTask();
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();
	bool delayed = config.useDelayedEvaluation();
	
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = LiftedEffectCSP::create_smart(actions, tuple_index, approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	SmartRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	
	return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, SmartRPG, GroundStateModel>(model, std::move(heuristic), delayed));
}

GroundStateModel
SmartEffectDriver::setup(const Config& config, Problem& problem) const {
	// We'll use all the ground actions for the search plus the partyally ground actions for the heuristic computations
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem);
}


} } // namespaces
