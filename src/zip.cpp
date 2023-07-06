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
#include "zip.hpp"

#include <fstream>
#include <vector>
#include <span>
#include <filesystem>
#include <exception>
#include <cstring>
#include <iostream>
#include <sstream>
#include <limits>
namespace libsc3
{
    libzip_failure::libzip_failure(const zip_error_t& error_struct)
    {
        err_struct = error_struct;
    }
    libzip_failure::libzip_failure(const int& err_int)
    {
        zip_error_init_with_code(&err_struct, err_int);
        zip_error_strerror(&err_struct);
    }
    libzip_failure::libzip_failure(zip_t* zp)
    {
        err_struct = *zip_get_error(zp);
    }
    libzip_failure::~libzip_failure()
    {
        zip_error_fini(&err_struct);
    }
    const char* libzip_failure::what() const noexcept
    {
        return zip_error_strerror(&err_struct);
    }
    zip::zip(const std::filesystem::path& input)
        : path(input)
    {
        int return_value = 0;
        zip_handle       = zip_open(path.c_str(), ZIP_RDONLY, &return_value);
        if (zip_handle == nullptr)
            throw libzip_failure(return_value);
    }
    zip::file zip::open(const std::string& path)
    {
        auto file_handle = zip_fopen(zip_handle, path.c_str(), 0);
        if (file_handle == nullptr)
            throw libzip_failure(zip_handle);
        try
            {
                return file(file_handle);
            }
        catch (const libzip_failure& fail)
            {
                throw libzip_failure(zip_handle);
            }
    }
    zip::file::file(zip_file_t* file)
        : file_handle(file)
    {
        auto buf = new char[READ_BLOCK_SIZE];

        zip_uint64_t written_size;
        do
            {
                written_size = zip_fread(file_handle, buf, READ_BLOCK_SIZE);
                if (written_size == std::numeric_limits<decltype(written_size)>::max())
                    throw libzip_failure();
                write(buf, written_size);
        } while (written_size < READ_BLOCK_SIZE);

        delete[] buf;
    }
}
