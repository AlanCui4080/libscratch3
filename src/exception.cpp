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
    errorstr = std::format(
        "libzip_error: {} (version: {})", zip_error_strerror(error),
        zip_libzip_version());
}
libzip_runtime_error::libzip_runtime_error(zip_t* p)
{
    error    = new zip_error_t;
    error    = zip_get_error(p);
    errorstr = std::format(
        "libzip_error: {} (version: {})", zip_error_strerror(error),
        zip_libzip_version());
}
libzip_runtime_error::libzip_runtime_error(zip_file_t* p)
{
    error    = new zip_error_t;
    error    = zip_file_get_error(p);
    errorstr = std::format(
        "libzip_error: {} (version: {})", zip_error_strerror(error),
        zip_libzip_version());
}
libzip_runtime_error::~libzip_runtime_error()
{
    zip_error_fini(error);
    delete error;
}
const char* libzip_runtime_error::what() const noexcept
{
    return errorstr.c_str();
}
file_format_error::file_format_error(
    const std::string& what, boost::json::value& va)
{
    ss_va << va;
    s_what = std::format("file_format_error: {} at: {}", what, ss_va.str());
}
const char* file_format_error::what() const noexcept
{
    return s_what.c_str();
}
const char* libsdl_runtime_error::what() const noexcept
{
    return SDL_GetError();
}
