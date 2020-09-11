//
// Created by Hobbyist Maker on 9/15/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DETECTPANELCOLLISIONS_HPP
#define SILVANUSPRO_DETECTPANELCOLLISIONS_HPP

#include "entities/DialogInputs.hpp"

using silvanus::generatebox::entities::JointPanelPlanes;
using silvanus::generatebox::entities::JointPanelPlanesParams;
using silvanus::generatebox::entities::DialogPanelCollisionPair;
using silvanus::generatebox::entities::DialogPanelCollisionPairParams;

auto detectPanelCollisionsImpl(const JointPanelPlanes& first, const JointPanelPlanes& second) -> DialogPanelCollisionPair;
auto detectPanelCollisionsParamsImpl(const JointPanelPlanesParams& first, const JointPanelPlanesParams& second) -> DialogPanelCollisionPairParams;

#endif //SILVANUSPRO_DETECTPANELCOLLISIONS_HPP
