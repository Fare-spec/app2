#ifndef GATEAU_REPLAY_H
#define GATEAU_REPLAY_H

#include <stdbool.h>
#include <stddef.h>

bool gateau_replay_candidate(const char *log_path, const char *trigger,
                             const char *input, const char *expected,
                             char *response, size_t capacity, size_t *step);

#endif
