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
#include <boost/json.hpp>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <zip.h>
namespace libsc3
{
    class libzip_runtime_error : public std::exception
    {
    private:
        zip_error_t* error;

    public:
        virtual const char* what() const noexcept override final;

    public:
        libzip_runtime_error(int zip_errorno);
        libzip_runtime_error(zip_t* p);
        libzip_runtime_error(zip_file_t* p);
        ~libzip_runtime_error();
    };
    class file_format_error : public std::exception
    {
    private:
        std::string       s_what;
        std::stringstream ss_va;

    public:
        virtual const char* what() const noexcept override final;

    public:
        file_format_error(const std::string& what, boost::json::value& va);
    };
    class libsdl_runtime_error : public std::exception
    {
    public:
        virtual const char* what() const noexcept override final;
    };
} // namespace libsc3
