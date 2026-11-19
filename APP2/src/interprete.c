#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "curiosity.h"
#include "listes.h"
#include "pile.h"

typedef struct {
  cellule_t *current;
  cellule_t *after;
} execution_frame_t;

/* Call stack of routines being executed, stored in one growable array so
 * that entering a routine does not need its own malloc/free. */
typedef struct {
  execution_frame_t *frames;
  size_t size;
  size_t capacity;
} frame_stack_t;

static bool push_frame(frame_stack_t *stack, cellule_t *first,
                       cellule_t *after) {
  if (stack->size == stack->capacity) {
    size_t capacity = stack->capacity == 0 ? 64 : stack->capacity * 2;
    if (capacity > SIZE_MAX / sizeof *stack->frames) {
      return false;
    }
    execution_frame_t *frames =
        realloc(stack->frames, capacity * sizeof *frames);
    if (frames == NULL) {
      return false;
    }
    stack->frames = frames;
    stack->capacity = capacity;
  }
  stack->frames[stack->size++] = (execution_frame_t){first, after};
  return true;
}

static bool execute_arithmetic(pile_t *stack, char op) {
  int right;
  int left;
  if (!pile_depiler_entier(stack, &right) ||
      !pile_depiler_entier(stack, &left)) {
    return false;
  }

  long long result;
  switch (op) {
  case '+': result = (long long)left + right; break;
  case '-': result = (long long)left - right; break;
  case '*': result = (long long)left * right; break;
  default: return false;
  }
  if (result < INT_MIN || result > INT_MAX) {
    return false;
  }
  return pile_empiler_entier(stack, (int)result);
}

static void report_stack_error(char command) {
  eprintf("Pile invalide ou mémoire insuffisante pendant la commande '%c'.\n",
          command);
}

int interprete(sequence_t *seq, bool debug) {
  if (seq == NULL) {
    return RATE;
  }

  pile_t stack;
  pile_init(&stack);
  frame_stack_t frames = {NULL, 0, 0};
  if (!push_frame(&frames, seq->tete, NULL)) {
    return RATE;
  }

  int status = CIBLERATEE;
  while (frames.size > 0) {
    execution_frame_t *frame = &frames.frames[frames.size - 1];
    if (frame->current == frame->after) {
      frames.size--;
      continue;
    }

    cellule_t *instruction = frame->current;
    frame->current = instruction->next;
    char command = instruction->command;

    if (command >= '0' && command <= '9') {
      if (!pile_empiler_entier(&stack, command - '0')) {
        report_stack_error(command);
        status = RATE;
        break;
      }
      continue;
    }

    switch (command) {
    case '{': {
      cellule_t *closing = brace_match(instruction);
      if (closing == NULL ||
          !pile_empiler_routine(&stack, instruction->next, closing)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      frame->current = closing->next;
      break;
    }

    case '}':
      eprintf("Accolade fermante sans ouverture.\n");
      status = RATE;
      goto finished;

    case 'A': {
      int result = avance();
      if (result == VICTOIRE) {
        status = VICTOIRE;
        goto finished;
      }
      if (result == RATE) {
        status = RATE;
        goto finished;
      }
      break;
    }

    case 'D': droite(); break;
    case 'G': gauche(); break;

    case 'M': {
      int direction;
      if (!pile_depiler_entier(&stack, &direction) ||
          !pile_empiler_entier(&stack, mesure(direction))) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;
    }

    case 'P': {
      int value;
      if (!pile_depiler_entier(&stack, &value)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      pose(value);
      break;
    }

    case '+':
    case '-':
    case '*':
      if (!execute_arithmetic(&stack, command)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;

    case '?': {
      pile_routine_t otherwise;
      pile_routine_t then;
      int condition;
      if (!pile_depiler_routine(&stack, &otherwise) ||
          !pile_depiler_routine(&stack, &then) ||
          !pile_depiler_entier(&stack, &condition) ||
          !push_frame(&frames,
                      condition != 0 ? then.first : otherwise.first,
                      condition != 0 ? then.after : otherwise.after)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;
    }

    case '!': {
      pile_routine_t routine;
      if (!pile_depiler_routine(&stack, &routine) ||
          !push_frame(&frames, routine.first, routine.after)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;
    }

    case 'B': {
      int count;
      pile_routine_t routine;
      if (!pile_depiler_entier(&stack, &count) ||
          !pile_depiler_routine(&stack, &routine)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      if (count > 0) {
        if (!pile_empiler_routine(&stack, routine.first, routine.after) ||
            !pile_empiler_entier(&stack, count - 1)) {
          report_stack_error(command);
          status = RATE;
          goto finished;
        }
        frame->current = instruction;
        if (!push_frame(&frames, routine.first, routine.after)) {
          report_stack_error(command);
          status = RATE;
          goto finished;
        }
      }
      break;
    }

    case 'X':
      if (!pile_echanger(&stack)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;

    case 'C':
      if (!pile_dupliquer(&stack)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;

    case 'I': {
      pile_valeur_t discarded;
      if (!pile_depiler(&stack, &discarded)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;
    }

    case 'Z':
      if (!pile_renverser(&stack)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;

    case 'R': {
      int steps;
      int count;
      if (!pile_depiler_entier(&stack, &steps) ||
          !pile_depiler_entier(&stack, &count) ||
          !pile_rotation(&stack, count, steps)) {
        report_stack_error(command);
        status = RATE;
        goto finished;
      }
      break;
    }

    default:
      eprintf("Commande inconnue : '%c'\n", command);
      status = RATE;
      goto finished;
    }

    if (!silent_mode) {
      afficherCarte();
      if (debug) {
        char enter = '\0';
        printf("Appuyer sur entrée pour continuer...\n");
        while (enter != '\r' && enter != '\n') {
          enter = getchar();
        }
      }
    }
  }

finished:
  free(frames.frames);
  pile_vider(&stack);
  return status;
}
