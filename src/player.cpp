#include "player.hpp"
#include "exception.hpp"
using namespace libsc3;
player::player(SDL_Window* window)
{
    this->player_renderer = SDL_CreateRenderer(window, -1, renderer_flag);
    if(this->player_renderer == nullptr)
    {
        throw sdl_error();
    } 
}
