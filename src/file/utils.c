#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

long get_file_size(FILE* file) {
    long old_position = ftell(file);

    fseek(file, 0, SEEK_END);
    long result = ftell(file);

    fseek(file, old_position, SEEK_SET);

    return result;
}

char* read_file(FILE* file, long* output_buffer_length, Error* error) {
    long text_buffer_length = get_file_size(file) + 1;
    *output_buffer_length  = text_buffer_length;

    char* text = (char*)malloc(text_buffer_length);

    fread(text, 1, text_buffer_length, file);
    if (ferror(file)) {
        *error = get_error_from_errno(errno);
        free(text);

        return NULL;
    }

    text[text_buffer_length - 1] = '\0';

    return text;
}

bool is_abs_path(const char* path) {
    return path[0] == '/';
}
