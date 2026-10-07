#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef NCURSES
#include <ncurses.h>
#endif
#include "listes.h"

bool silent_mode = false;
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

void convert(char *text, sq *sequence) {
  assert(text != NULL);
  assert(sequence != NULL);
  sequence->head = NULL;
  cellule_t **tail = &sequence->head;

  for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
    if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' ||
        *p == '\v') {
      continue;
    }

    cellule_t *node = new_cell();
    if (node == NULL) {
      while (sequence->head != NULL) {
        cellule_t *next = sequence->head->next;
        destroy_cell(sequence->head);
        sequence->head = next;
      }
      return;
    }
    node->command = (char)*p;
    *tail = node;
    tail = &node->next;
  }
}

void conversion(char *texte, sequence_t *seq) { convert(texte, seq); }

void display(sq *sequence) {
  assert(sequence != NULL);
  for (const cell *node = sequence->head; node != NULL; node = node->next) {
    putchar(node->command);
  }
}

void afficher(sequence_t *seq) { display(seq); }
