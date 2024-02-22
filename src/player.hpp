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
#include <memory>
namespace libsc3
{
    class player
    {
    public:
        static constexpr auto renderer_flag = SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC;

    private:
        SDL_Renderer* player_renderer;

    public:
        player();
        ~player();
        player(SDL_Window* window);
    };
} // namespace libsc3
