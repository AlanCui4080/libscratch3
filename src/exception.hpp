#pragma once
#include <exception>
#include <stdexcept>
#include <zip.h>
#include <boost/json.hpp>
#include <sstream>
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
        ~libzip_runtime_error();
    };
    class file_format_error : public std::exception
    {
        private:
        std::string s_what;
        std::stringstream ss_va;
        public:
        virtual const char* what() const noexcept override final;
        public:
        file_format_error(const std::string& what, boost::json::value& va);
    };
} // namespace libsc3
