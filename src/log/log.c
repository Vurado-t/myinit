#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "log.h"

void init_std_log(FILE* file, Error* error_buffer) {
    INIT_ERROR(error_buffer);

    int log_fd = fileno(file);
    dup2(log_fd, 1);
    dup2(log_fd, 2);
}

void log_info_msg(const char* fmt, ...) {
    printf("[INFO] ");

    va_list params;
    va_start(params, fmt);

    vprintf(fmt, params);

    va_end(params);

    printf("\n");

    fflush(stdout);
    fflush(stderr);
}

void log_error(Error error) {
    fprintf(stderr, "[ERROR] [%d] %s\n", error.code, error.message);
    fflush(stdout);
    fflush(stderr);
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

    fflush(stdout);
    fflush(stderr);
}

void log_config(const InitConfig* init_config) {
    printf("[INFO] [Config (Process Count: %lu; Source File: %s)]:\n",
           init_config->process_count,
           init_config->source_file_path);

    for (int i = 0; i < init_config->process_count; i++) {
        log_process_config("    ", &init_config->process_configs[i]);
        printf("\n");
    }

    fflush(stdout);
    fflush(stderr);
}
