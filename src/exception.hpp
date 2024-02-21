#pragma once
#include <exception>
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
        ~libzip_runtime_error();
    };
} // namespace libsc3
