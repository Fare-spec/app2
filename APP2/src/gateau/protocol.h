#ifndef GATEAU_PROTOCOL_H
#define GATEAU_PROTOCOL_H

#include <stdbool.h>

#define GATEAU_PAGE_CAPACITY 64
#define GATEAU_MIN_PAGE_LENGTH 3

enum gateau_state {
  GATEAU_SERVER_STATE,
  GATEAU_WELCOME,
  GATEAU_INTRODUCTION,
  GATEAU_PAGE,
  GATEAU_CAKE_WARNING,
  GATEAU_CAKE_FOUND,
  GATEAU_CAKE_STEPPED_ON,
  GATEAU_FINISHED,
  GATEAU_TANTALIZING_MESSAGE
};

bool gateau_extract_last_page(const char *response,
                              char page[GATEAU_PAGE_CAPACITY]);
enum gateau_state gateau_classify(const char *response,
                                  char page[GATEAU_PAGE_CAPACITY]);
bool gateau_should_stop(enum gateau_state state);
void gateau_describe(enum gateau_state state, const char *page);
bool gateau_reply_starts_exercise(const char *response);

#endif
