#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "log/log.h"
#include "file/utils.h"

void init_log() {
    Error error;
    INIT_ERROR(&error);

    FILE* log_file = fopen("/tmp/myinit.log", "a+");
    if (log_file == NULL) {
        log_error(get_error_from_errno(errno));
        exit(error.code);
    }

    init_std_log(log_file, &error);
    if (error.has_error) {
        log_error(get_error_from_errno(errno));
        exit(error.code);
    }

    if (fclose(log_file) != 0) {
        log_error(get_error_from_errno(errno));
        exit(error.code);
    }
}

void close_all_files() {
    for (int i = 0; i < FD_SETSIZE; i++)
        close(i);
}

void detach_terminal() {
    if (fork() != 0)
        exit(0);

    if (setsid() < 0)
        exit(errno);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        log_error_msg("No config file");
        return 1;
    }

    char* config_path = argv[1];
    if (!is_abs_path(config_path)) {
        log_error_msg("Config path must be absolute");
        return 1;
    }

    printf("Logs are in /tmp/myinit.log\n");

    if (chdir("/") != 0) {
        log_error_msg("chdir fail");
        return 1;
    }

    close_all_files();
    init_log();
    detach_terminal();

    Error error;

    InitConfig* config = load_config(config_path, &error);
    if (error.has_error) {
        log_error(error);
        return error.code;
    }

    log_config(config);

    while (true) {
        config = try_reload_config(config, &error);
        if (error.has_error){
            log_error(error);
            continue;
        }

        log_config(config);

        sleep(2);
    }


    return 0;
}
