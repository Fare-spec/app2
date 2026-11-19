#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

static const char *skip_space(const char *text) {
  while (isspace((unsigned char)*text)) {
    text++;
  }
  return text;
}

bool gateau_extract_last_page(const char *response,
                              char page[GATEAU_PAGE_CAPACITY]) {
  const char *page_start = NULL;
  size_t page_length = 0;

  for (const char *cursor = skip_space(response); *cursor != '\0';) {
    if (!isdigit((unsigned char)*cursor)) {
      cursor = skip_space(cursor + 1);
      continue;
    }

    const char *start = cursor;
    while (isdigit((unsigned char)*cursor)) {
      cursor++;
    }
    size_t length = (size_t)(cursor - start);
    if (length >= GATEAU_MIN_PAGE_LENGTH && length < GATEAU_PAGE_CAPACITY) {
      page_start = start;
      page_length = length;
    }
    cursor = skip_space(cursor);
  }

  if (page_start == NULL) {
    return false;
  }
  memcpy(page, page_start, page_length);
  page[page_length] = '\0';
  return true;
}

bool gateau_reply_starts_exercise(const char *response) {
  return strstr(response, "Will you find the cake?") != NULL;
}

enum gateau_state gateau_classify(const char *response,
                                  char page[GATEAU_PAGE_CAPACITY]) {
  if (strstr(response, "Bienvenue") == response) {
    return GATEAU_WELCOME;
  }
  if (gateau_reply_starts_exercise(response)) {
    return GATEAU_INTRODUCTION;
  }
  bool has_page = gateau_extract_last_page(response, page);

  if (strstr(response, "Finished in") == response) {
    return GATEAU_FINISHED;
  }
  if (strstr(response, "You found the cake!") != NULL &&
      strstr(response, "stepped on it") == NULL) {
    return GATEAU_CAKE_FOUND;
  }
  if (strstr(response, "stepped on it") != NULL ||
      strstr(response, "stepped-on cake") != NULL) {
    return GATEAU_CAKE_STEPPED_ON;
  }
  if (has_page && strstr(response, "be careful") != NULL) {
    return GATEAU_CAKE_WARNING;
  }
  if (has_page) {
    return GATEAU_PAGE;
  }
  return GATEAU_TANTALIZING_MESSAGE;
}

bool gateau_should_stop(enum gateau_state state) {
  return state == GATEAU_FINISHED || state == GATEAU_CAKE_FOUND ||
         state == GATEAU_TANTALIZING_MESSAGE;
}

void gateau_describe(enum gateau_state state, const char *page) {
  switch (state) {
  case GATEAU_CAKE_WARNING:
    printf("Warning: cake page %s; no automatic command sent\n", page);
    break;
  case GATEAU_CAKE_STEPPED_ON:
    printf("Cake stepped on: failure state reached\n");
    break;
  default:
    break;
  }
}
