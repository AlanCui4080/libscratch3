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
    return std::format("libzip_error: {}\nversion: {}", zip_error_strerror(error),
                       zip_libzip_version())
        .c_str();
}
