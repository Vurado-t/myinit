#ifndef MYINIT_PROCESS_MANAGER_H
#define MYINIT_PROCESS_MANAGER_H


#include "../config/config.h"

typedef struct {
    InitConfig* init_config;
    bool is_running;
} ProcessManager;

void watch(ProcessManager* manager);


#endif
