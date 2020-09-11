//
// Created by Hobbyist Maker on 9/17/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PROGRESSDIALOGCONTROL_HPP
#define SILVANUSPRO_PROGRESSDIALOGCONTROL_HPP

#include <Core/UserInterface/ProgressDialog.h>

namespace silvanus::generatebox::entities {

    struct ProgressDialogControl {
        adsk::core::Ptr<adsk::core::ProgressDialog> control;
    };

}

#endif //SILVANUSPRO_PROGRESSDIALOGCONTROL_HPP
