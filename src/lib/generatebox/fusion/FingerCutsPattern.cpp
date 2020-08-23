//
// Created by Hobbyist Maker on 8/4/20.
//

#include "FingerCutsPattern.hpp"

#include "entities/JointProfile.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <map>

#include "plog/Log.h"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;


adsk::core::Ptr<adsk::fusion::RectangularPatternFeature> FingerCutsPattern::copy(
    const DefaultModelingOrientations model_orientation,
    const std::vector<adsk::core::Ptr<adsk::fusion::ExtrudeFeature>>& features,
    const JointProfile& profile,
    const bool corner
) {
    Ptr<ObjectCollection> entities = adsk::core::ObjectCollection::create();

    for (const Ptr<ExtrudeFeature>& feature: features) {
        if (feature->isValid()) {
            entities->add(feature);
        }
    }

    auto axis_edges = m_axes_selector[model_orientation][profile.joint_orientation][profile.panel_orientation];
    auto pattern_distance = corner ? profile.corner_distance : profile.pattern_distance;
    auto finger_count = corner ? 2 : profile.finger_count;
    PLOG_DEBUG << "FingerCutsPattern: " << (int)profile.joint_orientation << ":" << (int)profile.panel_orientation;

    try {
        const Ptr<ConstructionAxis>& first_edge = axis_edges();

        Ptr<ValueInput> count = ValueInput::createByReal(finger_count);
        Ptr<ValueInput> distance = ValueInput::createByReal(pattern_distance);

        Ptr<RectangularPatternFeatures> patterns = m_component->features()->rectangularPatternFeatures();

        Ptr<RectangularPatternFeatureInput> pattern_input = patterns->createInput(
            entities, first_edge, count, distance, ExtentPatternDistanceType
        );

        return m_component->features()->rectangularPatternFeatures()->add(pattern_input);

    } catch(const std::runtime_error& re) {
            m_app->userInterface()->messageBox(std::to_string((int)profile.joint_orientation) + ":" + std::to_string((int)profile.panel_orientation));
            return nullptr;
    } catch(const std::exception &exc) {
            m_app->userInterface()->messageBox(std::to_string((int)profile.joint_orientation) + ":" + std::to_string((int)profile.panel_orientation));
            return nullptr;
    }
}
