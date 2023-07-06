/*
This file is part of libscratch3.

This program is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <fstream>
#include <vector>
#include <span>
#include <filesystem>
#include <exception>
#include <cstring>
#include <iostream>
#include <sstream>

#include <zip.h>
namespace libsc3
{
    class libzip_failure : public std::exception
    {
    private:
        mutable zip_error_t err_struct{};

    public:
        libzip_failure(const zip_error_t& error_struct);
        libzip_failure(const int& err_int);
        libzip_failure(zip_t* zp);
        libzip_failure() = default;
        ~libzip_failure();
        const char* what() const noexcept final;
    };
    class zip
    {
    public:
        class file : public std::stringstream
        {
        private:
            static constexpr auto READ_BLOCK_SIZE = 1024;

            zip_file_t* file_handle;

        public:
            file(zip_file_t* file);
        };

    private:
        const std::filesystem::path& path;
        zip_t*                       zip_handle;

    public:
        zip(const std::filesystem::path& input);
        file open(const std::string& path);
    };
}
