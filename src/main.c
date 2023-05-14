#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "log/log.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        log_error_msg("No config log_file");
        return 1;
    }

    Error error;

    FILE* log_file = fopen("/tmp/myinit.log", "a+");
    if (log_file == NULL) {
        log_error(get_error_from_errno(errno));
        return error.code;
    }

    init_std_log(log_file, &error);
    if (error.has_error) {
        log_error(get_error_from_errno(errno));
        return error.code;
    }

    if (fclose(log_file) != 0) {
        log_error(get_error_from_errno(errno));
        return error.code;
    }

    char* config_path = argv[1];

    InitConfig* config = load_config(config_path, &error);
    if (error.has_error) {
        log_error(error);
        return error.code;
    }

    log_config(config);

    char i;
    scanf("%c", &i);

    config = try_reload_config(config, &error);
    if (error.has_error) {
        log_error(error);
        return error.code;
    }

    log_config(config);

    free_config(config);

    return 0;
}
