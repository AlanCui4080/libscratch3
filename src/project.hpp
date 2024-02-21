#pragma once
#include <zip.h>
#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <boost/json.hpp>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
namespace libsc3
{
    class stage;
    class target
    {
        friend class stage;

    public:
        typedef std::variant<std::string, std::int64_t, double> variable_value_type;
        typedef SDL_Surface*                                    renderer_surface_type;
        typedef Mix_Music*                                      mixer_sound_type;
        typedef zip_file_t*                                     element_file_type;

    private:
        std::string_view name;
        std::unordered_map<std::string, std::pair<std::string, std::vector<variable_value_type>>>
                                                               variable_list;
        std::unordered_map<std::string, renderer_surface_type> costume_list;
        std::unordered_map<std::string, mixer_sound_type>      sound_list;

    private:
        virtual constexpr auto get_variable_list() noexcept -> decltype(variable_list)&
        {
            return variable_list;
        }

    public:
        target(stage& stage, boost::json::value& json_value,
               std::unordered_map<std::string, element_file_type>& elem_list);
    };

    class stage : public target
    {
    public:
        stage(boost::json::value&                                 json_value,
              std::unordered_map<std::string, element_file_type>& elem_list);
        virtual constexpr auto get_variable_list() noexcept -> decltype(variable_list)& override
        {
            return variable_list;
        }
    };

    class project
    {
    public:
        typedef zip_t*      project_bundle_type;
        typedef zip_file_t* element_file_type;

    private:
        boost::json::value                                 project_source;
        project_bundle_type                                compressed_bundle;
        std::unordered_map<std::string, element_file_type> element_list;
        std::unordered_map<std::string, target>            target_list;

        decltype(target_list)::iterator stage_target;

    public:
        project(const std::filesystem::path& path);
        ~project();
    };

} // namespace libsc3
