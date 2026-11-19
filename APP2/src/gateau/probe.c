#include <stdio.h>
#include <string.h>

#include "appolab.h"
#include "client.h"
#include "probe.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define KEY_CAPACITY 64
#define PAGE_CHAIN_LIMIT 100

static bool send(const char *input, char *response) {
  return appolab_send_command(input, response);
}

int gateau_probe_once(const char *phrase, size_t page_chain_limit) {
  char login[512];
  if (!appolab_read_login(login, sizeof login)) {
    return GATEAU_PROBE_ERROR;
  }

  char response[RESPONSE_CAPACITY];
  char page[KEY_CAPACITY];
  bool has_page = false;

  show_messages(false);
  if (!connexion(NULL) ||
      !send(login, response) ||
      !send("load leGateau", response) ||
      !send("depart", response)) {
    deconnexion();
    return GATEAU_PROBE_ERROR;
  }

  size_t digits = 0;
  for (const char *cursor = response; *cursor != '\0'; cursor++) {
    if (*cursor >= '0' && *cursor <= '9') {
      digits++;
    } else if (digits > 0) {
      if (digits >= 3 && digits < sizeof page) {
        memcpy(page, cursor - digits, digits);
        page[digits] = '\0';
        has_page = true;
      }
      digits = 0;
    }
  }
  if (digits >= 3 && digits < sizeof page) {
    memcpy(page, response + strlen(response) - digits, digits);
    page[digits] = '\0';
    has_page = true;
  }

  size_t chain_limit = page_chain_limit;
  while (has_page && chain_limit-- > 0) {
    if (!send(page, response)) {
      deconnexion();
      return GATEAU_PROBE_ERROR;
    }
    if (strstr(response, "be careful") != NULL) {
      if (!send(phrase, response)) {
        deconnexion();
        return GATEAU_PROBE_ERROR;
      }
      int result = GATEAU_PROBE_REJECTED;
      if (strstr(response, "I told you there was no cake...") == NULL) {
        result = GATEAU_PROBE_ACCEPTED;
      }
      deconnexion();
      return result;
    }

    digits = 0;
    has_page = false;
    for (const char *cursor = response; *cursor != '\0'; cursor++) {
      if (*cursor >= '0' && *cursor <= '9') {
        digits++;
      } else if (digits > 0) {
        if (digits >= 3 && digits < sizeof page) {
          memcpy(page, cursor - digits, digits);
          page[digits] = '\0';
          has_page = true;
        }
        digits = 0;
      }
    }
    if (digits >= 3 && digits < sizeof page) {
      memcpy(page, response + strlen(response) - digits, digits);
      page[digits] = '\0';
      has_page = true;
    }
  }

  deconnexion();
  return GATEAU_PROBE_NO_WARNING;
}
