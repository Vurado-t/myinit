#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#include "process_manager.h"
#include "../log/log.h"

#define MAX_PROCESSES_COUNT 100


pid_t fork_and_exec(const ProcessConfig* process_config) {
    pid_t child_pid = fork();
    switch (child_pid) {
        case -1:
            log_error_msg("Fork failed");
            break;
        case 0:
        {
            FILE* input_file = fopen(process_config->stdin_file, "r");
            if (input_file == NULL)
                exit(errno);

            FILE* output_file = fopen(process_config->stdout_file, "w");
            if (output_file == NULL)
                exit(errno);

            dup2(fileno(input_file), 0);

            int output_fd = fileno(output_file);
            dup2(output_fd, 1);
            dup2(output_fd, 2);

            char* exec_argv[process_config->argc + 2];
            exec_argv[0] = process_config->exe_path;
            for (size_t i = 1; i < process_config->argc + 1; i++)
                exec_argv[i] = process_config->argv[i];

            exec_argv[process_config->argc + 1] = NULL;

            for (size_t i = 0; i < process_config->argc + 2; i++) {
                printf(" %s", exec_argv[i]);
            }

            execv(process_config->exe_path, exec_argv);

            break;
        }
        default:
            break;
    }

    return child_pid;
}

/*
 * output_pids[i] matches to init_config.process_configs[i]
 * */
void start_children(const InitConfig* init_config, pid_t* output_pids) {
    for (size_t i = 0; i < init_config->process_count; i++) {
        output_pids[i] = fork_and_exec(&init_config->process_configs[i]);

        log_info_msg("Started number %d with pid %d: %s",
                     i,
                     output_pids[i],
                     init_config->process_configs[i].exe_path);
    }
}

void restart_by_index(size_t index, pid_t* pids, const InitConfig* init_config) {
    ProcessConfig* process_config = &init_config->process_configs[index];
    pids[index] = fork_and_exec(process_config);
}

void killall(pid_t* pids, size_t pids_length) {
    for (size_t i = 0; i < pids_length; i++) {
        if (kill(pids[i], SIGKILL) == 0) {
            log_info_msg("Sent kill signal to %d", pids[i]);
            continue;
        }

        log_info_msg("Can't kill pid: %d", pids[i]);
        log_error(get_error_from_errno(errno));
    }
}

void watch(ProcessManager* manager) {
    manager->is_running = true;

    pid_t pids[MAX_PROCESSES_COUNT];
    start_children(manager->init_config, pids);

    while (manager->is_running) {
        int status;
        pid_t pid = waitpid(-1, &status, 0);
        if (WIFEXITED(status))
            log_info_msg("Pid %d exited with status %d", pid, WEXITSTATUS(status));

        if (pid == -1) {
            log_error(get_error_from_errno(errno));
            break;
        }

        for (size_t i = 0; i < manager->init_config->process_count; i++) {
            if (pids[i] == pid) {
                log_info_msg("Finished number %d with pid %d", i, pid);

                restart_by_index(i, pids, manager->init_config);

                sleep(2);

                log_info_msg("Restarted number %d with new pid %d: %s",
                             i,
                             pids[i],
                             manager->init_config->process_configs[i].exe_path);
            }
        }
    }

    killall(pids, manager->init_config->process_count);
}
