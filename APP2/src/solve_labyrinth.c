#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "appolab.h"
#include "client.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define LOGIN_CAPACITY 512

static const char *solver_program = "{3M{G}{DA}?C!}C!";

static bool contains(const char *text, const char *part) {
  return strstr(text, part) != NULL;
}

static int submit_solver(void) {
  char login[LOGIN_CAPACITY];
  char response[RESPONSE_CAPACITY];
  if (!appolab_read_login(login, sizeof login)) {
    return EXIT_FAILURE;
  }

  show_messages(false);
  if (!connexion(NULL)) {
    return EXIT_FAILURE;
  }

  bool ok = appolab_send_command(login, response);
  if (ok && !contains(response, "Bienvenue")) {
    fprintf(stderr, "AppoLab did not accept the login\n");
    ok = false;
  }
  if (ok) {
    ok = appolab_send_command("load Labyrinthe", response);
  }
  if (ok) {
    ok = appolab_send_command("depart", response);
    if (ok && !contains(response, "code")) {
      fprintf(stderr, "AppoLab did not ask for a program\n");
      ok = false;
    }
  }
  if (ok) {
    ok = appolab_send_command(solver_program, response);
    if (ok && (contains(response, "échoué") || contains(response, "echec") ||
               contains(response, "plus d'instructions"))) {
      fprintf(stderr, "The submitted routine did not solve this maze\n");
      ok = false;
    }
  }

  deconnexion();
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--program") == 0) {
    puts(solver_program);
    return EXIT_SUCCESS;
  }
  if (argc == 2 && strcmp(argv[1], "--submit") == 0) {
    return submit_solver();
  }
  if (argc != 1) {
    fprintf(stderr, "Usage: %s [--program|--submit]\n", argv[0]);
    return EXIT_FAILURE;
  }
  fprintf(stderr, "Use --program for offline output or --submit to connect.\n");
  return EXIT_FAILURE;
}
