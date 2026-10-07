#ifndef LISTES_H
#define LISTES_H

#include <stdbool.h>
#include <stdio.h>

/*
 * Pour réaliser des tests de performance, désactiver tous les 
 * affichages.
 * Pour cela, le plus simple est de compiler avec la macro 'SILENT' définie, en 
 * faisant 'make silent'
 */

#ifdef SILENT

/* Desactive tous les affichages */
#define printf(fmt, ...) (0)
#define eprintf(fmt, ...) (0)
#define putchar(c) (0)
/* Desactive les 'Appuyer sur entrée pour continuer' */
#define getchar() ('\n')

#else

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

#endif

extern bool silent_mode;




struct cellule {
    char   command;
    /* vous pouvez rajouter d'autres champs ici */
    union {
        struct cellule *suivant; /* french name */
        struct cellule *next;    /* english name */
    };
};
typedef struct cellule cellule_t;
typedef struct cellule cell;

struct sequence {
    union {
        cellule_t *tete; /* french name */
        cellule_t *head; /* english name */
    };
};
typedef struct sequence sequence_t;
typedef struct sequence sq;


cell* new_cell (void);
void destroy_cell (cell*);
void convert (char *text, sq *sequence);
void display (sq *sequence);

cellule_t* nouvelleCellule (void);

void detruireCellule (cellule_t*);

void conversion (char *texte, sequence_t *seq);

void afficher (sequence_t* seq);


#endif
