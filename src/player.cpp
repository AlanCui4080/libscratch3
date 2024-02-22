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

#include "player.hpp"
#include "exception.hpp"

using namespace libsc3;
player::player()
{
    auto result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if (result < 0)
    {
        throw libsdl_runtime_error();
    }
    result = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG );
    if (result == 0)
    {
        throw libsdl_runtime_error();
    }
    result = Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG);
    if (result == 0)
    {
        throw libsdl_runtime_error();
    }
    result = Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096);
    if (result < 0)
    {
        throw libsdl_runtime_error();
    }
}
player::player(SDL_Window* window)
    : player()
{
    this->player_renderer = SDL_CreateRenderer(window, -1, renderer_flag);
    if (this->player_renderer == nullptr)
    {
        throw libsdl_runtime_error();
    }
}
player::~player()
{
    IMG_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}
