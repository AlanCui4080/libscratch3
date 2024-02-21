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

#include "project.hpp"
#include "exception.hpp"
#include <iostream>
#include <cstdlib>
using namespace libsc3;
project::project(const std::filesystem::path& path)
{
    int zip_errorno;
    this->compressed_bundle = zip_open(path.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &zip_errorno);
    if (this->compressed_bundle == nullptr)
    {
        throw libzip_runtime_error(zip_errorno);
    }

    auto element_count = zip_get_num_entries(this->compressed_bundle, 0);
    for (decltype(element_count) i = 0; i < element_count; i++)
    {
        zip_stat_t stat_data;
        auto       result = zip_stat_index(this->compressed_bundle, i, 0, &stat_data);
        if (result != 0)
        {
            throw libzip_runtime_error(this->compressed_bundle);
        }
        this->element_list.insert_or_assign(stat_data.name,
                                            zip_fopen_index(this->compressed_bundle, i, 0));
    }

    static constexpr auto buffer_block_step      = 4096;
    auto                  buffer_size            = buffer_block_step;
    auto                  project_element_buffer = std::calloc(buffer_size, 1);
    while (!zip_fread(this->element_list["project.json"], project_element_buffer, buffer_size))
    {
        buffer_size += 4096;
        project_element_buffer = std::realloc(project_element_buffer, buffer_size);
    }
    // it do make a copy, but pmr is too complex for now.
    this->project_source =
        decltype(this->project_source)(reinterpret_cast<char*>(project_element_buffer));
    std::free(project_element_buffer);

    this->stage_target = target_list.end();
    for (auto&& i : this->project_source.as_object()["targets"].as_array())
    {
        std::string_view target_name_view = i.as_object()["name"].as_string();
        if (i.as_object()["isStage"].as_bool())
        {
            auto stage_object = stage(i, this->element_list);
            auto result =
                this->target_list.emplace(std::string(target_name_view), std::move(stage_object));

            if (this->stage_target == target_list.end())
            {
                this->stage_target = result.first;
            }
            else
            {
                throw file_format_error("duplicated stage detected", i);
            }
        }
        else
        {
            if (this->stage_target == target_list.end())
            {
                throw file_format_error("a non stage target listed front of stage", i);
            }
            else
            {
                auto&& stage_ref     = dynamic_cast<stage&>((*this->stage_target).second);
                auto   target_object = target(stage_ref, i, this->element_list);
                this->target_list.emplace(std::string(target_name_view), std::move(target_object));
            }
        }
    }
}
project::~project()
{
    for (auto&& i : this->element_list)
    {
        zip_fclose(i.second);
    }
    zip_close(this->compressed_bundle);
}
static inline target::variable_value_type variable_value_helper(boost::json::value& va)
{
    if (va.is_int64())
    {
        return target::variable_value_type(va.as_int64());
    }
    else if (va.is_double())
    {
        return target::variable_value_type(va.as_double());
    }
    else if (va.is_string())
    {
        return target::variable_value_type(std::string(va.as_string()));
    }
    else
    {
        throw file_format_error("a variable is neither a number nor a string", va);
    }
}

target::target(stage& stage, boost::json::value& json_value,
               std::unordered_map<std::string, element_file_type>& elem_list)
{
    this->name = json_value.as_object()["name"].as_string();
    for (auto&& i : json_value.as_object()["variables"].as_object())
    {
        std::string_view key_view           = i.key();
        std::string_view variable_name_view = i.value().as_array()[0].as_string();

        auto value_pair = std::make_pair(std::string(variable_name_view),
                                        variable_value_helper(i.value().as_array()[1]));
        this->variable_list.insert_or_assign(std::string(key_view), std::move(value_pair));
    }
    for (auto&& i : json_value.as_object()["lists"].as_object())
    {
        std::string_view                 key_view           = i.key();
        std::string_view                 variable_name_view = i.value().as_array()[0].as_string();
        auto&&                           this_array         = i.value().as_array()[1].as_array();
        std::vector<variable_value_type> value_vector;
        std::for_each(this_array.begin(), this_array.end(), [&](decltype(*this_array.begin()) it) {
            value_vector.emplace_back(variable_value_helper(it));
        });
        auto value_pair = std::make_pair(std::string(variable_name_view), std::move(value_vector));
        this->list_list.insert_or_assign(std::string(key_view), std::move(value_pair));
    }
    for (auto&& i : json_value.as_object()["costumes"].as_array())
    {
        auto                  costume_name      = std::string(i.as_object()["name"].as_string());
        static constexpr auto buffer_block_step = 4096;
        auto                  buffer_size       = buffer_block_step;
        auto                  costume_buffer    = std::calloc(buffer_size, 1);
        while (!zip_fread(elem_list[std::string(i.as_object()["md5ext"].as_string())],
                          costume_buffer, buffer_size))
        {
            buffer_size += 4096;
            costume_buffer = std::realloc(costume_buffer, buffer_size);
        }
        auto costume_rw = SDL_RWFromMem(costume_buffer, buffer_size);
        if (costume_rw == nullptr)
        {
            throw sdl_error();
        }
        auto data_fmt_str    = i.as_object()["dataFormat"].as_string().c_str();
        auto costume_surface = IMG_LoadTyped_RW(costume_rw, 1, data_fmt_str);
        if (costume_surface == nullptr)
        {
            throw sdl_error();
        }
        costume_list.insert_or_assign(costume_name, costume_surface);
        std::free(costume_buffer);
    }
    for (auto&& i : json_value.as_object()["sounds"].as_array())
    {
        auto                  sound_name        = std::string(i.as_object()["name"].as_string());
        static constexpr auto buffer_block_step = 4096;
        auto                  buffer_size       = buffer_block_step;
        auto                  sound_buffer      = std::calloc(buffer_size, 1);
        while (!zip_fread(elem_list[std::string(i.as_object()["md5ext"].as_string())], sound_buffer,
                          buffer_size))
        {
            buffer_size += 4096;
            sound_buffer = std::realloc(sound_buffer, buffer_size);
        }
        auto sound_rw = SDL_RWFromMem(sound_buffer, buffer_size);
        if (sound_rw == nullptr)
        {
            throw sdl_error();
        }
        auto sound_target = Mix_LoadMUS_RW(sound_rw, 1);
        if (sound_target == nullptr)
        {
            throw sdl_error();
        }
        sound_list.insert_or_assign(sound_name, sound_target);
        std::free(sound_buffer);
    }
}
stage::stage(boost::json::value&                                 json_value,
             std::unordered_map<std::string, element_file_type>& elem_list)
    : target(*this, json_value, elem_list)
{
}
