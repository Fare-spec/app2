#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"

bool gateau_extract_code(const char *response,
                         char code[GATEAU_CODE_WIDTH + 1]) {
  for (size_t i = 0; response[i] != '\0'; i++) {
    if (isdigit((unsigned char)response[i]) &&
        (i == 0 || !isdigit((unsigned char)response[i - 1]))) {
      size_t j = 0;
      while (j < GATEAU_CODE_WIDTH && isdigit((unsigned char)response[i + j])) {
        j++;
      }
      if (j == GATEAU_CODE_WIDTH &&
          !isdigit((unsigned char)response[i + j])) {
        memcpy(code, response + i, GATEAU_CODE_WIDTH);
        code[GATEAU_CODE_WIDTH] = '\0';
        return true;
      }
    }
  }
  return false;
}

bool gateau_extract_number(const char *response, char *number, size_t capacity) {
  bool found = false;
  for (size_t i = 0; response[i] != '\0';) {
    if (!isdigit((unsigned char)response[i])) {
      i++;
      continue;
    }

    size_t start = i;
    while (isdigit((unsigned char)response[i])) {
      i++;
    }
    size_t length = i - start;
    if (length >= GATEAU_CODE_WIDTH && length < capacity) {
      memcpy(number, response + start, length);
      number[length] = '\0';
      found = true;
    }
  }
  return found;
}

bool gateau_reply_needs_manual_input(const char *response) {
  const char *needle = "careful";
  for (; *response != '\0'; response++) {
    size_t i = 0;
    while (needle[i] != '\0' && response[i] != '\0' &&
           tolower((unsigned char)response[i]) == needle[i]) {
      i++;
    }
    if (needle[i] == '\0') {
      return true;
    }
  }
  return false;
}

unsigned int gateau_code_index(const char code[GATEAU_CODE_WIDTH + 1]) {
  return (unsigned int)strtoul(code, NULL, 10);
}

void gateau_index_write(FILE *index_file, size_t step, const char *input,
                        const char *code, const char *response) {
  fprintf(index_file, "%zu\t%s\t%s\t", step, input, code == NULL ? "" : code);
  for (const unsigned char *p = (const unsigned char *)response; *p != '\0';
       p++) {
    fputc(*p == '\n' || *p == '\r' || *p == '\t' ? ' ' : *p, index_file);
  }
  fputc('\n', index_file);
  fflush(index_file);
}
