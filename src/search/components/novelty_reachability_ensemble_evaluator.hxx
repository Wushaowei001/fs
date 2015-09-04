
#pragma once

#include <fs0_types.hxx>
#include <state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <heuristics/novelty_from_preconditions.hxx>
#include "novelty_evaluator.hxx"

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace engines {
	
template <typename SearchNode>
class NoveltyReachabilityEnsembleEvaluator : public NoveltyEvaluator<SearchNode> {
	typedef NoveltyEvaluator<SearchNode> BaseClass;
	
public:
// 	typedef RelaxedPlanHeuristic<FS0StateModel> RelaxedHeuristic; // TODO - We need to decide here what type of RPG builder we'll be using here.
	//RelaxedHeuristic _reachability_heuristic;

	NoveltyReachabilityEnsembleEvaluator(const FS0StateModel& model) : BaseClass(model) {}

// 	float evaluate_reachability( const State& state ) {
// 		return _reachability_heuristic.evaluate( state );
// 	}

	float evaluate(const State& state) {
		auto novelty = BaseClass::novelty(state);
		if (novelty > BaseClass::novelty_bound()) return std::numeric_limits<float>::infinity();

		assert(0); // This needs to be properly implemented and clearly distinguished from gbfs(f)
		
// 		num_unsat = heuristic.evaluate_num_unsat_goals( state );
// 		SearchStatistics::instance().min_num_goals_sat = std::min( num_unsat, SearchStatistics::instance().min_num_goals_sat);
// 		if ( parent != nullptr && num_unsat < parent->num_unsat ) {
			//std::cout << "Reward!" << std::endl;
			//print(std::cout);
			//std::cout << std::endl;
// 		}
		/* NOT USED as this is the heuristic function for gbfs(f)
		h = heuristic.evaluate_reachability( state );
		unsigned ha = 2;
		if ( parent != nullptr && h < parent->h ) ha = 1;
		f = 2 * (novelty - 1) + ha;
		*/		
		
	}
};

} } // namespaces
