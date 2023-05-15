#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "log/log.h"
#include "file/utils.h"
#include "process_manager/process_manager.h"

ProcessManager manager;

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

    log_info_msg("Daemon started with pid: %d", getpid());
}

void handle_signals(int signum) {
    switch (signum) {
        case SIGHUP:
            manager.is_running = false;
            break;
        default:
            break;
    }
}

void config_signal_handling() {
    struct sigaction action;
    action.sa_handler = handle_signals;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGHUP, &action, NULL);
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
    config_signal_handling();

    Error error;

    InitConfig* config = load_config(config_path, &error);
    if (error.has_error) {
        log_error(error);
        return error.code;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        log_info_msg("Starting myinit");
        log_config(config);

        manager = (ProcessManager){.init_config = config, .is_running = false};

        watch(&manager);

        config = try_reload_config(config, &error);
        if (error.has_error)
            log_error(error);
    }
#pragma clang diagnostic pop
}
