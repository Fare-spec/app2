#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "appolab.h"
#include "client.h"
#include "gateau/protocol.h"

#define RESPONSE_CAPACITY (MAXREP + 1)

static bool submit(FILE *index_file, size_t *step, const char *input,
                   char *response, char page[GATEAU_PAGE_CAPACITY],
                   bool *has_page) {
  if (!appolab_send_command(input, response)) {
    return false;
  }
  enum gateau_state state = gateau_classify(response, page);
  *has_page = state == GATEAU_PAGE || state == GATEAU_CAKE_WARNING;
  fprintf(index_file, "%zu\t%s\t%s\t%c%s\n", *step, input,
          *has_page ? page : "", state + 'A', response);
  fflush(index_file);
  gateau_describe(state, page);
  (*step)++;
  if (gateau_should_stop(state)) {
    return false;
  }
  return true;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s output.tsv\n", argv[0]);
    return 1;
  }

  char login[512];
  if (!appolab_read_login(login, sizeof login)) {
    return 1;
  }
  FILE *index_file = fopen(argv[1], "w");
  if (index_file == NULL) {
    perror(argv[1]);
    return 1;
  }
  fputs("step\tinput\tpage\tstate\tserver_reply\n", index_file);

  char response[RESPONSE_CAPACITY];
  char page[GATEAU_PAGE_CAPACITY];
  bool has_page = false;
  size_t step = 0;
  bool running = true;
  char next_page[GATEAU_PAGE_CAPACITY];

  show_messages(false);
  if (!connexion(NULL)) {
    fclose(index_file);
    return 1;
  }
  running = submit(index_file, &step, login, response, page, &has_page) &&
            submit(index_file, &step, "load leGateau", response, page,
                   &has_page) &&
            submit(index_file, &step, "start", response, page, &has_page);

  if (running && has_page) {
    snprintf(next_page, sizeof next_page, "%s", page);
  } else {
    running = false;
  }

  while (running) {
    char current_page[GATEAU_PAGE_CAPACITY];
    snprintf(current_page, sizeof current_page, "%s", next_page);
    running = submit(index_file, &step, current_page, response, page,
                     &has_page);
    if (running && has_page) {
      snprintf(next_page, sizeof next_page, "%s", page);
    } else {
      running = false;
    }
    sleep(1);
  }

  deconnexion();
  fclose(index_file);
  return 0;
}
