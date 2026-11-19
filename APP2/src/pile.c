#include <stdint.h>
#include <stdlib.h>
#include "pile.h"

static bool pile_agrandir(pile_t *pile) {
  if (pile->taille < pile->capacite) {
    return true;
  }

  size_t nouvelle_capacite = pile->capacite == 0 ? 64 : pile->capacite * 2;
  if (nouvelle_capacite < pile->capacite ||
      nouvelle_capacite > SIZE_MAX / sizeof *pile->valeurs) {
    return false;
  }

  pile_valeur_t *nouvelles_valeurs =
      realloc(pile->valeurs, nouvelle_capacite * sizeof *pile->valeurs);
  if (nouvelles_valeurs == NULL) {
    return false;
  }
  pile->valeurs = nouvelles_valeurs;
  pile->capacite = nouvelle_capacite;
  return true;
}

void pile_init(pile_t *pile) {
  if (pile != NULL) {
    pile->valeurs = NULL;
    pile->taille = 0;
    pile->capacite = 0;
  }
}

static bool pile_empiler_valeur(pile_t *pile, pile_valeur_t valeur) {
  if (pile == NULL || !pile_agrandir(pile)) {
    return false;
  }
  pile->valeurs[pile->taille++] = valeur;
  return true;
}

bool pile_empiler_entier(pile_t *pile, int valeur) {
  pile_valeur_t element = {.type = PILE_ENTIER, .valeur.entier = valeur};
  return pile_empiler_valeur(pile, element);
}

void pile_empiler(pile_t *pile, int valeur) {
  (void)pile_empiler_entier(pile, valeur);
}

bool pile_empiler_routine(pile_t *pile, cellule_t *first,
                          cellule_t *after) {
  pile_valeur_t element = {
      .type = PILE_ROUTINE,
      .valeur.routine = {.first = first, .after = after},
  };
  return pile_empiler_valeur(pile, element);
}

bool pile_depiler(pile_t *pile, pile_valeur_t *valeur) {
  if (pile == NULL || pile->taille == 0) {
    return false;
  }
  pile_valeur_t element = pile->valeurs[--pile->taille];
  if (valeur != NULL) {
    *valeur = element;
  }
  return true;
}

bool pile_depiler_entier(pile_t *pile, int *valeur) {
  if (pile == NULL || pile->taille == 0 ||
      pile->valeurs[pile->taille - 1].type != PILE_ENTIER) {
    return false;
  }
  int element = pile->valeurs[--pile->taille].valeur.entier;
  if (valeur != NULL) {
    *valeur = element;
  }
  return true;
}

bool pile_depiler_routine(pile_t *pile, pile_routine_t *routine) {
  if (pile == NULL || pile->taille == 0 ||
      pile->valeurs[pile->taille - 1].type != PILE_ROUTINE) {
    return false;
  }
  pile_routine_t element = pile->valeurs[--pile->taille].valeur.routine;
  if (routine != NULL) {
    *routine = element;
  }
  return true;
}

bool pile_sommet(const pile_t *pile, pile_valeur_t *valeur) {
  if (pile == NULL || pile->taille == 0) {
    return false;
  }
  if (valeur != NULL) {
    *valeur = pile->valeurs[pile->taille - 1];
  }
  return true;
}

bool pile_dupliquer(pile_t *pile) {
  if (pile == NULL || pile->taille == 0) {
    return false;
  }
  return pile_empiler_valeur(pile, pile->valeurs[pile->taille - 1]);
}

bool pile_echanger(pile_t *pile) {
  if (pile == NULL || pile->taille < 2) {
    return false;
  }
  size_t top = pile->taille - 1;
  pile_valeur_t temp = pile->valeurs[top];
  pile->valeurs[top] = pile->valeurs[top - 1];
  pile->valeurs[top - 1] = temp;
  return true;
}

static void pile_inverser(pile_valeur_t *values, size_t first, size_t last) {
  while (first < last) {
    pile_valeur_t temp = values[first];
    values[first++] = values[last];
    values[last--] = temp;
  }
}

bool pile_rotation(pile_t *pile, int count, int steps) {
  if (pile == NULL || count < 0 || (size_t)count > pile->taille) {
    return false;
  }
  if (count == 0) {
    return true;
  }

  steps %= count;
  if (steps < 0) {
    steps += count;
  }
  if (steps == 0) {
    return true;
  }

  size_t first = pile->taille - (size_t)count;
  size_t last = pile->taille - 1;
  pile_inverser(pile->valeurs, first, last);
  pile_inverser(pile->valeurs, first, last - (size_t)steps);
  pile_inverser(pile->valeurs, last - (size_t)steps + 1, last);
  return true;
}

/* Commande 'Z' : renverse toute la pile (le sommet devient le fond). */
bool pile_renverser(pile_t *pile) {
  if (pile == NULL) {
    return false;
  }
  if (pile->taille > 1) {
    pile_inverser(pile->valeurs, 0, pile->taille - 1);
  }
  return true;
}

bool pile_est_vide(const pile_t *pile) {
  return pile == NULL || pile->taille == 0;
}

void pile_vider(pile_t *pile) {
  if (pile == NULL) {
    return;
  }
  free(pile->valeurs);
  pile->valeurs = NULL;
  pile->taille = 0;
  pile->capacite = 0;
}
