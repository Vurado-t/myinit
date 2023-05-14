#ifndef LCK_ERROR_HANDLING_H
#define LCK_ERROR_HANDLING_H


#include <stdbool.h>

#define INIT_ERROR(error) *error = (Error){.has_error = false, .inner = NULL}

typedef struct Error Error;

struct Error {
    bool has_error;
    int code;
    const char* message;
    Error* inner;
};

Error get_error_from_errno(int errno_value);

Error get_error_from_message(const char* message);


#endif
