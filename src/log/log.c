#include <stdio.h>
#include <unistd.h>
#include "log.h"

void init_std_log(FILE* file, Error* error_buffer) {
    INIT_ERROR(error_buffer);

    int log_fd = fileno(file);
    dup2(log_fd, 1);
    dup2(log_fd, 2);
}

void log_error(Error error) {
    fprintf(stderr, "%s [%d]\n", error.message, error.code);
}

void log_error_msg(char* msg) {
    log_error(get_error_from_message(msg));
}

void log_process_config(char* message_before, const ProcessConfig* process_config) {
    printf("%s[%s]", message_before, process_config->exe_path);
    for (int j = 0; j < process_config->argc; j++)
        printf(" [%s]", process_config->argv[j]);

    printf(" [%s]", process_config->stdin_file);
    printf(" [%s]", process_config->stdout_file);
}

void log_config(const InitConfig* init_config) {
    printf("[Config (processes: %lu)]:\n", init_config->process_count);

    for (int i = 0; i < init_config->process_count; i++) {
        log_process_config("    ", &init_config->process_configs[i]);
        printf("\n");
    }
}
