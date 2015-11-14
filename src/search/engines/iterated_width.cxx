
#include <search/engines/iterated_width.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <actions/applicable_action_set.hxx>

namespace fs0 { namespace engines {

std::unique_ptr<FS0SearchAlgorithm> IteratedWidthEngineCreator::create(const Config& config, const FS0StateModel& model) const {
	
	unsigned max_novelty = config.getOption<int>("engine.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	FINFO("main", "Heuristic options:");
	FINFO("main", "\tMax novelty: " << max_novelty);
	FINFO("main", "\tFeatiue extaction: " << feature_configuration);
	
	FS0SearchAlgorithm* engine = new FS0IWAlgorithm(model, 1, max_novelty, feature_configuration);
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

} } // namespaces
