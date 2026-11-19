#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "replay.h"

static char *read_file(const char *path, size_t *length) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    return NULL;
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return NULL;
  }
  long size = ftell(file);
  if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return NULL;
  }
  char *data = malloc((size_t)size + 1);
  if (data == NULL || fread(data, 1, (size_t)size, file) != (size_t)size) {
    free(data);
    fclose(file);
    return NULL;
  }
  fclose(file);
  data[size] = '\0';
  *length = (size_t)size;
  return data;
}

bool gateau_replay_candidate(const char *log_path, const char *trigger,
                             const char *input, const char *expected,
                             char *response, size_t capacity,
                             size_t *step) {
  size_t length = 0;
  char *text = read_file(log_path, &length);
  if (text == NULL) {
    return false;
  }

  char *line = strtok(text, "\n");
  bool found = false;
  *step = 0;
  while (line != NULL) {
    char *end = NULL;
    unsigned long number = strtoul(line, &end, 10);
    if (end != line && *end == '\t') {
      char *record_input = end + 1;
      char *response_start = strchr(record_input, '\t');
      if (response_start != NULL) {
        *response_start++ = '\0';
        if (strstr(response_start, trigger) != NULL) {
          *step = (size_t)number;
          snprintf(response, capacity, "%s", input);
          found = strstr(response, expected) != NULL;
          break;
        }
      }
    }
    line = strtok(NULL, "\n");
  }

  free(text);
  return found;
}
