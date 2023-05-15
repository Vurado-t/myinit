#include <bits/types/FILE.h>
#include "../error/error.h"
#include "../config/config.h"

#ifndef MYINIT_LOG_H
#define MYINIT_LOG_H


void init_std_log(FILE* stream, Error* error_buffer);

void log_error(Error error);

void log_error_msg(char* msg);

void log_process_config(char* message_before, const ProcessConfig* process_config);

void log_config(const InitConfig* init_config);

void log_info_msg(const char* fmt, ...);

#endif
