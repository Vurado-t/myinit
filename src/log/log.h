#include <bits/types/FILE.h>
#include "../error/error.h"
#include "../config/config.h"

#ifndef MYINIT_LOG_H
#define MYINIT_LOG_H


void init_stdout_log(FILE* stream, Error* error_buffer);

void log_error(Error error);

void log_error_msg(char* msg);

void log_config(const InitConfig* init_config);

#endif
