// This file is a part of libscratch3
// Copyright (C) 2024 libscratch3 Authors

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <boost/json.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <zip.h>
namespace libsc3
{
    class stage;
    class target
    {
        friend class stage;

    public:
        // a container to express scratch *any* type variables
        typedef std::variant<std::string, std::int64_t, double>
                             variable_value_type;
        typedef SDL_Surface* renderer_surface_type;
        typedef Mix_Music*   mixer_sound_type;
        typedef zip_file_t*  element_file_type;

    private:
        stage&           stage_reference;
        std::string_view name;

        // store all objects in "variables"
        std::unordered_map<
            std::string, std::pair<std::string, variable_value_type>>
                                                          variable_list;
        // store all objects in "sounds"
        std::unordered_map<std::string, mixer_sound_type> sound_list;
        // store all objects in "lists"
        std::unordered_map<
            std::string,
            std::pair<std::string, std::vector<variable_value_type>>>
                                                               list_list;
        // store all objects in "costumes"
        std::unordered_map<std::string, renderer_surface_type> costume_list;

    public:
        /**
         * @brief constructor
         *
         * @param stage stage reference providing visibility for name looking up
         * @param json_value an value presenting this target
         * @param elem_list compressed file list fpr media loading
         */
        target(
            stage& stage, boost::json::value& json_value,
            std::unordered_map<std::string, element_file_type>& elem_list);

            target(
             boost::json::value& json_value,
            std::unordered_map<std::string, element_file_type>& elem_list);
        ~target();
    };

    class stage : public target
    {
    public:
        /**
         * @brief constructor
         *
         * @param json_value an value presenting this target
         * @param elem_list compressed file list fpr media loading
         *
         * @note only one stage can be constructed in a project, it's actually a
         * warpper to target::target which provide *this to stage reference.
         */
        stage(
            boost::json::value&                                 json_value,
            std::unordered_map<std::string, element_file_type>& elem_list);

    public:
        auto get_variable_list() -> decltype(variable_list)&;
    };

    class project
    {
    public:
        typedef zip_t*      project_bundle_type;
        typedef zip_file_t* element_file_type;

    private:
        boost::json::object                                project_source;
        project_bundle_type                                compressed_bundle;
        std::unordered_map<std::string, element_file_type> element_list;
        std::unordered_map<std::string, target>            target_list;
        decltype(target_list)::iterator                    stage_target;

    public:
        /**
         * @brief constructor
         *
         * @param path an value presenting the .sb3 file
         */
        project(const std::filesystem::path& path);
        ~project();
    };

} // namespace libsc3
