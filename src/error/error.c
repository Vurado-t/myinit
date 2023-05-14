#include "error.h"
#include <string.h>

Error get_error_from_errno(int errno_value) {
    Error error;
    INIT_ERROR(&error);

    error.has_error = errno_value != 0;
    error.code = errno_value;
    error.message = strerror(errno_value);

    return error;
}

Error get_error_from_message(const char* message) {
    Error error;
    INIT_ERROR(&error);

    error.has_error = true;
    error.code = 1;
    error.message = message;

    return error;
}
