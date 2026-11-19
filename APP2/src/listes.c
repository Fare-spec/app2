#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef NCURSES
#include <ncurses.h>
#endif
#include "listes.h"

bool silent_mode = false;

typedef struct {
  cellule_t cellule;
  cellule_t *matching;
} cellule_ouvrante_t;
/*
 * I will probably rewrite some of this with typdef that are similar with Rust's
 * one because its confusing
 * like u32, i32, String etc...
 */

cell *new_cell(void) {
  cell *result = malloc(sizeof *result);
  if (result == NULL) {
    return NULL;
  }

  result->command = '\0';
  result->next = NULL;
  return result;
}
cellule_t *nouvelleCellule(void) { return new_cell(); }

void detruireCellule(cellule_t *cel) { destroy_cell(cel); }

void destroy_cell(cell *node) { free(node); }

cellule_t *brace_match(cellule_t *opening) {
  if (opening == NULL || opening->command != '{') {
    return NULL;
  }
  return ((cellule_ouvrante_t *)opening)->matching;
}

static bool is_blank(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
         c == '\v';
}

/*
 * Builds the linked list in two passes over the text: the first counts the
 * cells so that they can all be placed in one allocation (instead of one
 * malloc/free per command), the second links them and pairs the braces.
 */
void convert(char *text, sq *sequence) {
  assert(text != NULL);
  assert(sequence != NULL);
  sequence->head = NULL;
  sequence->storage = NULL;

  size_t commands = 0;
  size_t openings = 0;
  for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
    if (!is_blank(*p)) {
      commands++;
      openings += (*p == '{');
    }
  }
  if (commands == 0) {
    return;
  }
  if (commands > SIZE_MAX / sizeof(cellule_ouvrante_t)) {
    return;
  }

  size_t bytes = (commands - openings) * sizeof(cellule_t) +
                 openings * sizeof(cellule_ouvrante_t);
  unsigned char *storage = malloc(bytes);
  cellule_t **pending = NULL;
  if (storage == NULL ||
      (openings > 0 && (pending = malloc(openings * sizeof *pending)) == NULL)) {
    free(storage);
    return;
  }

  unsigned char *free_space = storage;
  cellule_t **tail = &sequence->head;
  size_t depth = 0;
  bool balanced = true;

  for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
    if (is_blank(*p)) {
      continue;
    }

    cellule_t *node = (cellule_t *)free_space;
    if (*p == '{') {
      ((cellule_ouvrante_t *)node)->matching = NULL;
      pending[depth++] = node;
      free_space += sizeof(cellule_ouvrante_t);
    } else {
      if (*p == '}') {
        if (depth == 0) {
          balanced = false;
          break;
        }
        ((cellule_ouvrante_t *)pending[--depth])->matching = node;
      }
      free_space += sizeof(cellule_t);
    }
    node->command = (char)*p;
    *tail = node;
    tail = &node->next;
  }
  *tail = NULL;
  free(pending);

  if (!balanced || depth != 0) {
    free(storage);
    sequence->head = NULL;
    return;
  }
  sequence->storage = storage;
}

void conversion(char *texte, sequence_t *seq) { convert(texte, seq); }

void display(sq *sequence) {
  assert(sequence != NULL);
  for (const cell *node = sequence->head; node != NULL; node = node->next) {
    putchar(node->command);
  }
}

void afficher(sequence_t *seq) { display(seq); }

int is_empty(sequence_t *seq) { return (seq->head == NULL); }

void clear_sequence(sequence_t *seq) {
  if (seq == NULL) {
    return;
  }
  if (seq->storage != NULL) {
    free(seq->storage);
    seq->storage = NULL;
    seq->head = NULL;
    return;
  }
  cellule_t *node = seq->head;
  while (node != NULL) {
    cellule_t *next = node->next;
    destroy_cell(node);
    node = next;
  }
  seq->head = NULL;
}
