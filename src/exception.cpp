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

#include "exception.hpp"
#include <format>
using namespace libsc3;
libzip_runtime_error::libzip_runtime_error(int zip_errorno)
{
    error = new zip_error_t;
    zip_error_init_with_code(error, zip_errorno);
}
libzip_runtime_error::libzip_runtime_error(zip_t* p)
{
    error = new zip_error_t;
    error = zip_get_error(p);
}
libzip_runtime_error::~libzip_runtime_error()
{
    zip_error_fini(error);
    delete error;
}
const char* libzip_runtime_error::what() const noexcept
{
    return std::format("libzip_error: {}\nversion: {}",
                       zip_error_strerror(error), zip_libzip_version())
        .c_str();
}
file_format_error::file_format_error(const std::string&  what,
                                     boost::json::value& va)
{
    s_what = what;
    ss_va << va;
}
const char* file_format_error::what() const noexcept
{
    return std::format("file_format_error: {} \n at: {}", s_what, ss_va.str())
        .c_str();
}
const char* sdl_error::what() const noexcept
{
    return SDL_GetError();
}
