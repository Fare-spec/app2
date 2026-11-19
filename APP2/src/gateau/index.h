#ifndef GATEAU_INDEX_H
#define GATEAU_INDEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define GATEAU_CODE_WIDTH 5
#define GATEAU_CODE_COUNT 100000

bool gateau_extract_code(const char *response,
                         char code[GATEAU_CODE_WIDTH + 1]);
bool gateau_extract_number(const char *response, char *number, size_t capacity);
bool gateau_reply_needs_manual_input(const char *response);
unsigned int gateau_code_index(const char code[GATEAU_CODE_WIDTH + 1]);
void gateau_index_write(FILE *index_file, size_t step, const char *input,
                        const char *code, const char *response);

#endif
