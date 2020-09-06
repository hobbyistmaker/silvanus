//
// Created by Hobbyist Maker on 8/5/20.
//

#ifndef SILVANUSPRO_RENDERER_HPP
#define SILVANUSPRO_RENDERER_HPP

#include "rendersupport.hpp"

namespace silvanus::generatebox::render {

    class Renderer {

    protected:
        axis_transform_map yup_sketch_transform = { // NOLINT(cert-err58-cpp)
            {entities::AxisFlag::Height, [](double x, double y, double z){
                return std::make_tuple(x, -y, z);
            }},
            {entities::AxisFlag::Length, [](double x, double y, double z){
                return std::make_tuple(-x, y, z);
            }},
            {entities::AxisFlag::Width, [](double x, double y, double z){
                return std::make_tuple(x, y, z);
            }}
        };
        axis_transform_map zup_sketch_transform = { // NOLINT(cert-err58-cpp)
            {entities::AxisFlag::Height, [](double x, double y, double z){
                return std::make_tuple(x, y, z);
            }},
            {entities::AxisFlag::Length, [](double x, double y, double z){
                return std::make_tuple(-y, x, z);
            }},
            {entities::AxisFlag::Width, [](double x, double y, double z){
                return std::make_tuple(x, -y, z);
            }}
        };
        orientation_transform_map sketch_transforms = { // NOLINT(cert-err58-cpp)
            {adsk::core::YUpModelingOrientation, yup_sketch_transform},
            {adsk::core::ZUpModelingOrientation, zup_sketch_transform}
        };

        static std::string concat_names(const std::vector<std::string>& source) {
            if (source.empty()) return "";

            auto names = std::string{source[0]};

            if (source.size() < 2) {return names;}

            for (const auto &name : std::vector<std::string>{source.begin() + 1, source.end()}) names += "-" + name;
            return names;
        }

    public:
        virtual void execute(
            adsk::core::DefaultModelingOrientations orientation,
            const adsk::core::Ptr<adsk::fusion::Component>& component
        ) = 0;
    };

}

#endif //SILVANUSPRO_RENDERER_HPP
