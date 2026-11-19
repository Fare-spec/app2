#ifndef PILE_H
#define PILE_H

#include <stdbool.h>
#include <stddef.h>
#include "listes.h"

typedef struct {
  cellule_t *first;
  cellule_t *after;
} pile_routine_t;

typedef enum {
  PILE_ENTIER,
  PILE_ROUTINE
} pile_type_t;

typedef struct {
  pile_type_t type;
  union {
    int entier;
    pile_routine_t routine;
  } valeur;
} pile_valeur_t;

typedef struct {
  pile_valeur_t *valeurs;
  size_t taille;
  size_t capacite;
} pile_t;

void pile_init(pile_t *pile);
void pile_empiler(pile_t *pile, int valeur);
bool pile_empiler_entier(pile_t *pile, int valeur);
bool pile_empiler_routine(pile_t *pile, cellule_t *first,
                          cellule_t *after);
bool pile_depiler(pile_t *pile, pile_valeur_t *valeur);
bool pile_depiler_entier(pile_t *pile, int *valeur);
bool pile_depiler_routine(pile_t *pile, pile_routine_t *routine);
bool pile_sommet(const pile_t *pile, pile_valeur_t *valeur);
bool pile_dupliquer(pile_t *pile);
bool pile_echanger(pile_t *pile);
bool pile_rotation(pile_t *pile, int count, int steps);
bool pile_renverser(pile_t *pile);
bool pile_est_vide(const pile_t *pile);
void pile_vider(pile_t *pile);

#endif
