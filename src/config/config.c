#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "../file/utils.h"

#define MAX_LEXEME_IN_ROW_COUNT 100

typedef struct {
    const char* str;
    long begin;
    long end;
} Lexeme;

typedef struct {
    long length;
    Lexeme* lexemes;
} ParsedRow;

bool equals(const Lexeme* lexeme, const char* str, int number) {
    return strncmp(lexeme->str + lexeme->begin, str, number) == 0;
}

char* get_string_from_lexeme(const Lexeme* lexeme) {
    bool has_quote = lexeme->str[lexeme->begin] == '"';
    long begin = lexeme->begin + has_quote;
    long end = lexeme->end - has_quote;

    long buffer_length = end - begin + 1;
    char* buffer = malloc(sizeof(char) * buffer_length);
    buffer[buffer_length - 1] = '\0';

    return strncpy(buffer, lexeme->str + begin, buffer_length - 1);
}

long count_not_empty_rows(const char* buffer, long buffer_length) {
    if (buffer_length < 2)
        return 0;

    long count = 0;
    bool has_symbol = false;

    for (long i = 0; i < buffer_length; i++) {
        if (buffer[i] != '\n' && buffer[i] != '\0') {
            has_symbol = true;
            continue;
        }

        if (has_symbol) {
            has_symbol = false;
            count++;
        }
    }

    return count;
}

/*
 * "<str>" is lexeme
 * \n is lexeme
 * returns lexeme with 0 length when no lexemes
 * */
Lexeme get_next_lexeme(const char* buffer, long buffer_length, long offset, Error* error) {
    INIT_ERROR(error);

    long offset_without_whitespaces = offset;
    while (buffer[offset_without_whitespaces] == ' ' || buffer[offset_without_whitespaces] == '\t')
        offset_without_whitespaces++;

    Lexeme lexeme = (Lexeme){.str = buffer, .begin = offset_without_whitespaces};
    bool has_open_quote = false;
    for (long i = offset_without_whitespaces; i < buffer_length; i++) {
        if (has_open_quote) {
            if (buffer[i] == '"') {
                has_open_quote = false;
                lexeme.end = i + 1;
                break;
            }

            continue;
        }

        if (buffer[i] == '"') {
            has_open_quote = true;
            continue;
        }

        if (buffer[i] == ' ' || buffer[i] == '\0') {
            lexeme.end = i;
            break;
        }

        if (buffer[i] == '\n') {
            if (i == offset_without_whitespaces) {
                lexeme.end = i + 1;
                break;
            }

            lexeme.end = i;
            break;
        }
    }

    if (has_open_quote)
        *error = get_error_from_message("No close quote");

    return lexeme;
}

void free_parsed_rows(ParsedRow* parsed_rows, long length) {
    for (long i = 0; i < length; i++)
        free(parsed_rows[i].lexemes);

    free(parsed_rows);
}

ParsedRow* get_lexemes(char* text, long text_buffer_length, long row_count, long max_lexeme_in_row_count, Error* error) {
    INIT_ERROR(error);

    ParsedRow* parsed_rows = (ParsedRow*)malloc(sizeof(ParsedRow) * row_count);
    for (int i = 0; i < row_count; i++) {
        parsed_rows[i].length = 0;
        parsed_rows[i].lexemes = (Lexeme*)malloc(sizeof(Lexeme) * max_lexeme_in_row_count);
    }

    Lexeme current_lexeme = (Lexeme){.str = NULL};
    long current_row = 0;

    while (true) {
        current_lexeme = get_next_lexeme(text, text_buffer_length, current_lexeme.end, error);
        if (error->has_error) {
            free_parsed_rows(parsed_rows, row_count);
            return NULL;
        }

        long lexeme_length = current_lexeme.end - current_lexeme.begin;
        if (lexeme_length == 0)
            break;

        if (equals(&current_lexeme, "\n", 1)) {
            current_row++;
            continue;
        }

        parsed_rows[current_row].lexemes[parsed_rows[current_row].length++] = current_lexeme;
    }

    return parsed_rows;
}

ProcessConfig get_process_config(ParsedRow parsed_row, Error* error) {
    INIT_ERROR(error);

    if (parsed_row.length < 3) {
        *error = get_error_from_message("No executable file either I/O files");
        return (ProcessConfig) {};
    }

    ProcessConfig config;
    config.exe_path = get_string_from_lexeme(&parsed_row.lexemes[0]);
    config.stdin_file = get_string_from_lexeme(&parsed_row.lexemes[parsed_row.length - 2]);
    config.stdout_file = get_string_from_lexeme(&parsed_row.lexemes[parsed_row.length - 1]);

    config.argc = (int)(parsed_row.length - 3);
    config.argv = malloc(sizeof(char*) * config.argc);

    for (long i = 1; i < parsed_row.length - 2; i++)
        config.argv[i - 1] = get_string_from_lexeme(&parsed_row.lexemes[i]);

    return config;
}

InitConfig* parse_config_file(FILE* file, Error* error) {
    INIT_ERROR(error);

    long text_buffer_length;
    char* text = read_file(file, &text_buffer_length, error);
    if (error->has_error)
        return NULL;

    long row_count = count_not_empty_rows(text, text_buffer_length);
    ProcessConfig* process_configs = (ProcessConfig*)malloc(sizeof(ProcessConfig) * row_count);

    ParsedRow* parsed_rows = get_lexemes(text, text_buffer_length, row_count, MAX_LEXEME_IN_ROW_COUNT, error);

    if (error->has_error) {
        free(text);
        return NULL;
    }

    for (long i = 0; i < row_count; i++) {
        process_configs[i] = get_process_config(parsed_rows[i], error);
        if (error->has_error) {
            free_parsed_rows(parsed_rows, i + 1);
            free(text);

            return NULL;
        }
    }

    free_parsed_rows(parsed_rows, row_count);
    free(text);

    InitConfig* init_config = (InitConfig*)malloc(sizeof(InitConfig));
    init_config->process_count = row_count;
    init_config->process_configs = process_configs;

    return init_config;
}

void free_config(InitConfig* init_config) {
    for (int i = 0; i < init_config->process_count; i++) {
        ProcessConfig* process_config = &init_config->process_configs[i];

        for (int j = 0; j < process_config->argc; j++)
            free(process_config->argv[j]);

        free(process_config->argv);
        free(process_config->exe_path);
        free(process_config->stdin_file);
        free(process_config->stdout_file);
    }

    free(init_config->source_file_path);
    free(init_config->process_configs);
    free(init_config);
}

bool is_valid_config(InitConfig* init_config, Error* error) {
    INIT_ERROR(error);

    for (int i = 0; i < init_config->process_count; i++) {
        const ProcessConfig* process_config = &init_config->process_configs[i];

        if (!is_abs_path(process_config->exe_path))
            *error = get_error_from_message("Executable path must be absolute");

        if (!is_abs_path(process_config->stdin_file))
            *error = get_error_from_message("Stdin path must be absolute");

        if (!is_abs_path(process_config->stdout_file))
            *error = get_error_from_message("Stdout path must be absolute");

        if (error->has_error)
            return false;
    }

    return true;
}

InitConfig* load_config(const char* source_path, Error* error) {
    FILE* file = fopen(source_path, "r");
    if (file == NULL) {
        *error = get_error_from_errno(errno);
        return NULL;
    }

    InitConfig* config = parse_config_file(file, error);

    if (is_abs_path(source_path))
        config->source_file_path = strcpy(malloc(sizeof(char) * (FILENAME_MAX + 1)), source_path);
    else
        config->source_file_path = realpath(source_path, malloc(sizeof(char) * (FILENAME_MAX + 1)));

    if (error->has_error) {
        if (fclose(file) != 0)
            *error = get_error_from_errno(errno);

        return NULL;
    }

    if (!is_valid_config(config, error)) {
        if (fclose(file) != 0)
            *error = get_error_from_errno(errno);

        free_config(config);

        return NULL;
    }

    return config;
}

/*
 * [Description]
 *     free init_config and returns new config if there is no error, otherwise returns init_config
 * */
InitConfig* try_reload_config(InitConfig* init_config, Error* error) {
    INIT_ERROR(error);

    InitConfig* new_config = load_config(init_config->source_file_path, error);
    if (error->has_error)
        return init_config;

    free_config(init_config);

    return new_config;
}
