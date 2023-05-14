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
    char* source_file_path;
} InitConfig;

InitConfig* parse_config_file(FILE* file, Error* error);

void free_config(InitConfig* init_config);

bool is_valid_config(InitConfig* init_config, Error* error);

InitConfig* load_config(const char* source_path, Error* error);

InitConfig* try_reload_config(InitConfig* init_config, Error* error);


#endif
