#include <bits/types/FILE.h>
#include "../error/error.h"

#ifndef MYINIT_CONFIG_H
#define MYINIT_CONFIG_H


typedef struct {
    char* exe_path;
    int argc;
    char** argv;
    char* stdin_file;
    char* stdout_file;
} ProcessConfig;

typedef struct {
    long process_count;
    ProcessConfig* process_configs;
} InitConfig;

InitConfig* parse_config_file(FILE* file, Error* error);

void free_config(InitConfig* init_config);


#endif
