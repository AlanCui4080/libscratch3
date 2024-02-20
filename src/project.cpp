#include "project.hpp"
#include <iostream>
#include <cstdlib>
using namespace libsc3;
[[noreturn]] static inline void libzip_error_helper(int zip_errorno)
{
    zip_error_t error;
    zip_error_init_with_code(&error, zip_errorno);
    std::cerr << zip_error_strerror(&error) << std::endl;
    zip_error_fini(&error);
    std::exit(EXIT_FAILURE);
}
[[noreturn]] static inline void libzip_error_helper(zip_t* p)
{
    auto error = zip_get_error(p);
    std::cerr << zip_error_strerror(error) << std::endl;
    zip_error_fini(error);
    std::exit(EXIT_FAILURE);
}
project::project(const std::filesystem::path& path)
{
    int zip_errorno;
    this->compressed_bundle = zip_open(path.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &zip_errorno);
    if (this->compressed_bundle == nullptr)
    {
        libzip_error_helper(zip_errorno);
    }

    auto element_count = zip_get_num_entries(this->compressed_bundle, 0);
    for (decltype(element_count) i = 0; i < element_count; i++)
    {
        zip_stat_t stat_data;
        auto       result = zip_stat_index(this->compressed_bundle, i, 0, &stat_data);
        if (result != 0)
        {
            libzip_error_helper(this->compressed_bundle);
        }
        this->element_list.insert_or_assign(stat_data.name, zip_fopen_index(this->compressed_bundle, i, 0));
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
    this->project_source = decltype(this->project_source)(reinterpret_cast<char*>(project_element_buffer));
    std::free(project_element_buffer);

    this->stage_target = target_list.end();
    for (auto&& i : this->project_source.as_object()["targets"].as_array())
    {
        std::string_view target_name_view = i.as_object()["name"].as_string();
        if (i.as_object()["isStage"].as_bool())
        {
            auto stage_object = stage(i);
            auto result = this->target_list.insert_or_assign(std::string(target_name_view), std::move(stage_object));

            if (this->stage_target == target_list.end())
            {
                this->stage_target = result.first;
            }
            else
            {
                // TODO: better error handling
                std::cerr << "duplicated stage detected" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        else
        {
            this->target_list.insert_or_assign(std::string(target_name_view), i);
        }
    }
}
project::~project()
{
    zip_close(this->compressed_bundle);
    for (auto&& i : this->element_list)
    {
        zip_fclose(i.second);
    }
}
target::target(boost::json::value& json_value)
{
    this->name = json_value.as_object()["name"].as_string();
    for (auto&& i : json_value.as_object()["variables"].as_object())
    {
        std::string_view key_view           = i.key();
        std::string_view variable_name_view = i.value().as_array()[0].as_string();

        auto value_pair = std::make_pair(std::string(variable_name_view), i.value().as_array()[1].as_int64());
        this->variable_list.insert_or_assign(std::string(key_view), std::move(value_pair));
    }
}
stage::stage(boost::json::value& json_value)
    : target(json_value)
{
}
