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
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace libsc3;
namespace detail
{
    class file_segment : public std::pair<void*, int>
    {
    public:
        file_segment(void* p, int s)
            : std::pair<void*, int>(p, s)
        {
        }
        ~file_segment()
        {
            std::free(this->first);
        }
    };
} // namespace detail

/**
 * @brief helper to read a compressed file to memory
 *
 * @param file_to_read
 * @retval .first = pointer to buffer read out
 * @retval .second = size of effective content length
 */
static inline detail::file_segment
libzip_file_read_helper(project::element_file_type file_to_read)
{
    static constexpr auto buffer_block_step   = 4096;
    auto                  buffer_size         = 0;
    void*                 buffer              = nullptr;
    std::uint64_t         total_read_out_size = 0;
    std::uint64_t         read_out_size;
    do
    {
        buffer_size += buffer_block_step;
        const auto new_buffer = std::realloc(buffer, buffer_size);
        if (new_buffer == nullptr)
        {
            std::free(new_buffer);
            throw std::system_error(
                errno, std::generic_category(), "failed to realloc buffer");
        }
        buffer = new_buffer;
        read_out_size = zip_fread(file_to_read, buffer, buffer_size);
        if (read_out_size == -1U)
        {
            throw libzip_runtime_error(file_to_read);
        }
        total_read_out_size += read_out_size;
    } while (read_out_size != 0);
    assert(total_read_out_size < std::numeric_limits<int>::max());
    return { buffer, static_cast<int>(total_read_out_size) };
}

project::project(const std::filesystem::path& path)
{
    int zip_errorno;
    this->compressed_bundle =
        zip_open(path.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &zip_errorno);
    if (this->compressed_bundle == nullptr)
    {
        throw libzip_runtime_error(zip_errorno);
    }

    auto element_count = zip_get_num_entries(this->compressed_bundle, 0);
    for (decltype(element_count) i = 0; i < element_count; i++)
    {
        zip_stat_t stat_data;
        auto result = zip_stat_index(this->compressed_bundle, i, 0, &stat_data);
        if (result != 0)
        {
            throw libzip_runtime_error(this->compressed_bundle);
        }
        this->element_list.insert_or_assign(
            stat_data.name, zip_fopen_index(this->compressed_bundle, i, 0));
    }

    // read entire project.json out.
    auto file_to_read = this->element_list["project.json"];
    auto file_buffer  = libzip_file_read_helper(file_to_read);
    // it do make a copy, but pmr is too complex for now.
    this->project_source = decltype(this->project_source)(
        static_cast<char*>(file_buffer.first));

    this->stage_target = target_list.end();
    for (auto&& i : this->project_source.as_object()["targets"].as_array())
    {
        std::string_view target_name_view = i.as_object()["name"].as_string();
        if (i.as_object()["isStage"].as_bool())
        {
            auto stage_object = stage(i, this->element_list);
            auto result       = this->target_list.emplace(
                std::string(target_name_view), std::move(stage_object));

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
            if (this->stage_target != target_list.end())
            {
                auto&& stage_ref =
                    static_cast<stage&>(this->stage_target->second);
                auto target_object = target(stage_ref, i, this->element_list);
                this->target_list.emplace(
                    std::string(target_name_view), std::move(target_object));
            }
            else
            {
                throw file_format_error(
                    "a non stage target listed front of stage", i);
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
static inline target::variable_value_type
variable_value_helper(boost::json::value& va)
{
    if (va.is_int64())
    {
        return {va.as_int64()};
    }
    else if (va.is_double())
    {
        return {va.as_double()};
    }
    else if (va.is_string())
    {
        return {std::string(va.as_string())};
    }
    else
    {
        throw file_format_error(
            "a variable is neither a number nor a string", va);
    }
}

target::target(
    stage& stage, boost::json::value& json_value,
    std::unordered_map<std::string, element_file_type>& elem_list)
    : stage_reference(stage)
{
    this->name = json_value.as_object()["name"].as_string();

    // FORMAT EXAMPLE:
    //
    // "variables": {
    //     "`jEk@4|i[#Fk?(8x)AV": [
    //         "my variable",
    //         0
    //     ]
    // },
    for (auto&& i : json_value.as_object()["variables"].as_object())
    {
        std::string_view key_view = i.key();
        std::string_view variable_name_view =
            i.value().as_array()[0].as_string();

        auto value_pair = std::make_pair(
            std::string(variable_name_view),
            variable_value_helper(i.value().as_array()[1]));

        this->variable_list.insert_or_assign(
            std::string(key_view), std::move(value_pair));
    }

    // FORMAT EXAMPLE:
    //
    // "lists": {
    //     "0V;K4aNBrP1#^NS(HgFN": [
    //         "my list",
    //         [
    //             "D",
    //             "F",
    //             "J",
    //             "K"
    //         ]
    //     ]
    // },
    for (auto&& i : json_value.as_object()["lists"].as_object())
    {
        std::string_view key_view = i.key();
        std::string_view variable_name_view =
            i.value().as_array()[0].as_string();

        auto&& this_array = i.value().as_array()[1].as_array();

        std::vector<variable_value_type> value_vector;
        std::for_each(
            this_array.begin(), this_array.end(),
            [&](decltype(*this_array.begin()) it) {
                value_vector.emplace_back(variable_value_helper(it));
            });

        auto value_pair = std::make_pair(
            std::string(variable_name_view), std::move(value_vector));
        this->list_list.insert_or_assign(
            std::string(key_view), std::move(value_pair));
    }

    // FORMAT EXAMPLE:
    //
    // "currentCostume": 0,
    // "costumes": [
    //     {
    //         "name": "backdrop1",
    //         "dataFormat": "svg",
    //         "assetId": "cd21514d0531fdffb22204e0ec5ed84a", ///ignored.
    //         "md5ext": "cd21514d0531fdffb22204e0ec5ed84a.svg",
    //         "rotationCenterX": 240,
    //         "rotationCenterY": 180
    //     }
    // ],
    for (auto&& i : json_value.as_object()["costumes"].as_array())
    {
        auto costume_name = std::string(i.as_object()["name"].as_string());
        auto file_to_read =
            elem_list[std::string(i.as_object()["md5ext"].as_string())];
        auto file_buffer = libzip_file_read_helper(file_to_read);


        auto costume_rw  = SDL_RWFromMem(file_buffer.first, file_buffer.second);
        if (costume_rw == nullptr)
        {
            throw libsdl_runtime_error();
        }

        auto data_fmt_str = i.as_object()["dataFormat"].as_string().c_str();
        // number "1" in arguments presenting to free RWpos when returning. so
        // no need for SDL_RWclose below
        auto costume_surface = IMG_LoadTyped_RW(costume_rw, 1, data_fmt_str);
        if (costume_surface == nullptr)
        {
            throw libsdl_runtime_error();
        }
        costume_list.insert_or_assign(costume_name, costume_surface);
    }

    // FORMAT EXAMPLE:
    //
    // "sounds": [
    //     {
    //         "name": "pop",
    //         "assetId": "83a9787d4cb6f3b7632b4ddfebf74367", ///ignored.
    //         "dataFormat": "wav",
    //         "format": "",
    //         "rate": 48000,
    //         "sampleCount": 1123,
    //         "md5ext": "83a9787d4cb6f3b7632b4ddfebf74367.wav"
    //     }
    // ],
    // "volume": 100,
    for (auto&& i : json_value.as_object()["sounds"].as_array())
    {
        auto sound_name = std::string(i.as_object()["name"].as_string());
        auto file_to_read =
            elem_list[std::string(i.as_object()["md5ext"].as_string())];
        auto file_buffer = libzip_file_read_helper(file_to_read);
        auto sound_rw    = SDL_RWFromMem(file_buffer.first, file_buffer.second);
        if (sound_rw == nullptr)
        {
            throw libsdl_runtime_error();
        }
        // number "1" in arguments presenting to free RWpos when returning. so
        // no need for SDL_RWclose below
        auto sound_target = Mix_LoadMUS_RW(sound_rw, 1);
        if (sound_target == nullptr)
        {
            throw libsdl_runtime_error();
        }
        sound_list.insert_or_assign(sound_name, sound_target);
    }
}
stage::stage(
    boost::json::value&                                 json_value,
    std::unordered_map<std::string, element_file_type>& elem_list)
    : target(*this, json_value, elem_list)
{
}
auto stage::get_variable_list() -> decltype(variable_list)&
{
    return variable_list;
}
