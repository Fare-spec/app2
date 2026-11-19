#ifndef GATEAU_FLOW_H
#define GATEAU_FLOW_H

#include <stdbool.h>
#include <stddef.h>

#define PATTERN_COUNT (sizeof(patterns) / sizeof(patterns[0]))
#define MAX_FLOW_RESPONSE 100001

bool gateau_flow_extract_key(const char *response, char *key, size_t capacity);
bool gateau_flow_prepare(void);

#endif
