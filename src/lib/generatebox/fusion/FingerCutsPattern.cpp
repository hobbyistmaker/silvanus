//
// Created by Hobbyist Maker on 8/4/20.
//

#include "FingerCutsPattern.hpp"

#include "entities/JointProfile.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <map>

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;


adsk::core::Ptr<adsk::fusion::RectangularPatternFeature> FingerCutsPattern::copy(
    const DefaultModelingOrientations model_orientation,
    std::vector<adsk::core::Ptr<adsk::fusion::ExtrudeFeature>>& features,
    JointProfile& profile
) {
    Ptr<ObjectCollection> entities = adsk::core::ObjectCollection::create();

    for (const Ptr<ExtrudeFeature>& feature: features) {
        if (feature->isValid()) {
            entities->add(feature);
        }
    }

    auto axis_edges = m_axes_selector[model_orientation][profile.joint_orientation][profile.panel_orientation];

    try {
        const Ptr<ConstructionAxis>& first_edge = axis_edges();

        Ptr<ValueInput> count = ValueInput::createByReal(profile.finger_count);
        Ptr<ValueInput> distance = ValueInput::createByReal(profile.pattern_distance);

        Ptr<RectangularPatternFeatures> patterns = m_component->features()->rectangularPatternFeatures();

        Ptr<RectangularPatternFeatureInput> pattern_input = patterns->createInput(
            entities, first_edge, count, distance, ExtentPatternDistanceType
        );

        return m_component->features()->rectangularPatternFeatures()->add(pattern_input);

    } catch(const std::runtime_error& re) {
            m_app->userInterface()->messageBox(re.what());
            return nullptr;
    } catch(const std::exception &exc) {
            m_app->userInterface()->messageBox(exc.what());
            return nullptr;
    }
}
