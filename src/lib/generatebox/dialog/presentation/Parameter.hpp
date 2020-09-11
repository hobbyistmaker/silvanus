//
// Created by Hobbyist Maker on 9/19/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGPARAMETER_HPP
#define SILVANUSPRO_DIALOGPARAMETER_HPP

#include <fmt/format.h>
#include <string>
#include <utility>

namespace silvanus::generatebox::dialog {
    class DialogParameter {

        public:
            virtual void update(std::string value) {};
            virtual void update(double value) {};
    };

    class ParameterReference : DialogParameter {

            std::string m_reference;

        public:
            ParameterReference(std::string reference) : m_reference{std::move(reference)} {};
    };

    class ParameterValue : DialogParameter {

            std::string m_value;

        public:
            ParameterValue(double value) {
                m_value = std::to_string(value);
            };
    };

    class ParameterExpression : DialogParameter {
            std::string m_formula;

        public:
            ParameterExpression(std::string formula) : m_formula{formula} {};
            void update(double value) {
                fmt::format(m_formula, value);
            }
    };
}

#endif //SILVANUSPRO_PARAMETER_HPP
