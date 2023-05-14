#include <stdio.h>
#include <errno.h>
#include "log/log.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        log_error_msg("No config file");
        return 1;
    }

    char* config_path = argv[1];

    Error error;

    FILE* file = fopen(config_path, "r");
    if (file == NULL) {
        error = get_error_from_errno(errno);
        log_error(error);
        return error.code;
    }

    InitConfig* config = parse_config_file(file, &error);
    if (error.has_error) {
        log_error(error);
        return error.code;
    }

    log_config(config);

    free_config(config);

    if (fclose(file) != 0) {
        error = get_error_from_errno(errno);
        log_error(error);
        return error.code;
    }

    return 0;
}
