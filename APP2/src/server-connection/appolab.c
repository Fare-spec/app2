#include <stdio.h>
#include <string.h>

#include "appolab.h"
#include "client.h"

#define LOGIN_LINE_CAPACITY 512

bool appolab_read_login(char *login, size_t capacity) {
  const char *path = ".env";
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    path = "../.env";
    file = fopen(path, "r");
  }
  if (file == NULL) {
    perror(path);
    return false;
  }

  bool found = false;
  char line[LOGIN_LINE_CAPACITY];
  while (fgets(line, sizeof line, file) != NULL) {
    size_t length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
      line[--length] = '\0';
    }
    if (strncmp(line, "login ", 6) == 0) {
      if (length < capacity) {
        memcpy(login, line, length + 1);
        found = true;
      }
      break;
    }
  }

  fclose(file);
  if (!found) {
    fprintf(stderr, "No valid 'login ...' command found in .env\n");
  }
  return found;
}

bool appolab_send_command(const char *command, char *response) {
  size_t length = strlen(command);
  if (length > MAXREP) {
    fprintf(stderr, "Command is too long\n");
    return false;
  }

  char message[MAXREP + 1];
  memcpy(message, command, length + 1);
  if (envoyer_recevoir(message, response) < 0) {
    fprintf(stderr, "Connection to AppoLab was lost\n");
    return false;
  }

  if (response[0] != '\0') {
    fputs(response, stdout);
    if (response[strlen(response) - 1] != '\n') {
      putchar('\n');
    }
  }
  return true;
}
