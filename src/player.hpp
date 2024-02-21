#pragma once
#include <SDL2/SDL.h>
#include <memory>
namespace libsc3
{
    class player
    {
        public:
        static constexpr auto renderer_flag = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
        private:
        SDL_Renderer* player_renderer;
        public:
        player(SDL_Window* window);
    };
} // namespace libsc3
