#include "curiosity.h"
#include "listes.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct carte mars;
Robot curiosity;

void initCarte(int carte_num) {
  mars.carte_num = carte_num;
  mars.hauteur = 0;
  mars.largeur = 0;
  for (int j = 0; j < tailleCarte; j++) {
    for (int i = 0; i < tailleCarte; i++) {
      mars.map[j][i] = '\0';
      mars.marques[j][i] = false;
    }
  }
  init_robot(&curiosity, -1, -1, Est);
}

void ajoutLigneCarte(char *ligne) {
  assert(mars.hauteur < tailleCarte);

  if (mars.hauteur == 0) {
    mars.largeur = strlen(ligne) - 1;
  } else {
    assert(mars.largeur == strlen(ligne) - 1);
  }

  for (unsigned int i = 0; i < mars.largeur; i++) {
    if (ligne[i] == 'C') {
      mars.map[mars.hauteur][i] = PLAIN;
      if (curiosity.x != -1 || curiosity.y != -1) {
        eprintf("Curiosity déjà placée sur la carte!\n");
        exit(1);
      }
      init_robot(&curiosity, (int)i, (int)mars.hauteur, Est);
    } else if (ligne[i] == 'M') {
      mars.map[mars.hauteur][i] = PLAIN;
      mars.marques[mars.hauteur][i] = true;
    } else if (ligne[i] == 'P') {
      mars.map[mars.hauteur][i] = PLAIN;
      mars.marques[mars.hauteur][i] = true;
      init_robot(&curiosity, (int)i, (int)mars.hauteur, Est);
    } else {
      mars.map[mars.hauteur][i] = ligne[i];
    }
  }
  mars.hauteur++;
}

int char_to_color(char c);

void afficherCarte() {
  unsigned int i, j;
  char c;

  printf("\n\n\n\n\n\nCarte de mars.\n");
  printf("--------------\n\n\n\n");

  for (j = 0; j < mars.hauteur; j++) {

    for (i = 0; i < mars.largeur; i++) {
      c = mars.map[j][i];
      if (c == '\0') {
        if (i == 0) {
          j = tailleCarte;
          break;
        }
        break;
      }
      if ((int)i == curiosity.x && (int)j == curiosity.y) {
        if (curiosity.o == Est) {
          putchar('>');
        } else if (curiosity.o == Ouest) {
          putchar('<');
        } else if (curiosity.o == Sud) {
          putchar('V');
        } else {
          putchar('^');
        }
      } else {
        putchar(c);
      }
    }
    printf("\n");
  }
  printf("\n\n\n\n--------------\n\n");
  return;
}

bool verifieMarques(void) {
  unsigned int i, j;
  bool erreur = false;

  for (j = 0; j < mars.hauteur; j++) {
    for (i = 0; i < mars.largeur; i++) {
      if (!((mars.map[j][i] == MARK) == (mars.marques[j][i]))) {
        if (!erreur) {
          eprintf("******************************************\n");
          eprintf("*        ERREUR                          *\n");
          eprintf("* marque(s) manquante(s) ou en trop!     *\n");
          erreur = true;
        }
        eprintf("- erreur de marque position: %dx%d\n", i, j);
      }
    }
  }
  return !erreur;
}

int avance(void) {
  int tmpX;
  int tmpY;
  position_devant(&curiosity, &tmpX, &tmpY);

  if (tmpX < 0 || tmpX >= (int)mars.largeur || tmpY < 0 ||
      tmpY >= (int)mars.hauteur) {
    eprintf("Sortie de carte !!!\n");
    return RATE;
  }

  char c = mars.map[tmpY][tmpX];

  if (c == WATER) {
    eprintf("Plouf !?!\n");
    return RATE;
  }
  if (c == ROCK) {
    eprintf("Ouch!!!\n");
    return RATE;
  }
  avancer(&curiosity);
  if (c == TARGET) {

    if (!silent_mode) {
      printf("**************************\n");
      printf("*                        *\n");
      printf("*     Victoire !!!       *\n");
      printf("*                        *\n");
      printf("**************************\n\n\n");
    }
    return VICTOIRE;
  }
  if ((c != '.') && (c != 'M') && (c != 'm')) {
    printf("Oups ! Curiosity est sur '%c'\n\n", c);
    return RATE;
  }
  return REUSSI;
}

void droite() {
  tourner_a_droite(&curiosity);
}

void gauche() {
  tourner_a_gauche(&curiosity);
}

char charMesure(int dir) {
  Robot forward = curiosity;
  Robot right = curiosity;
  avancer(&forward);
  tourner_a_droite(&right);
  avancer(&right);
  int forward_x = forward.x - curiosity.x;
  int forward_y = forward.y - curiosity.y;
  int right_x = right.x - curiosity.x;
  int right_y = right.y - curiosity.y;
  int offset_x;
  int offset_y;

  switch (dir) {
  case 0:
    offset_x = 0;
    offset_y = 0;
    break;
  case 1:
    offset_x = forward_x;
    offset_y = forward_y;
    break;
  case 2:
    offset_x = forward_x + right_x;
    offset_y = forward_y + right_y;
    break;
  case 3:
    offset_x = right_x;
    offset_y = right_y;
    break;
  case 4:
    offset_x = -forward_x + right_x;
    offset_y = -forward_y + right_y;
    break;
  case 5:
    offset_x = -forward_x;
    offset_y = -forward_y;
    break;
  case 6:
    offset_x = -forward_x - right_x;
    offset_y = -forward_y - right_y;
    break;
  case 7:
    offset_x = -right_x;
    offset_y = -right_y;
    break;
  case 8:
    offset_x = forward_x - right_x;
    offset_y = forward_y - right_y;
    break;
  default:
    eprintf("Direction inconnue: %d\n", dir);
    assert(false);
    return ROCK;
  }

  long long x = (long long)curiosity.x + offset_x;
  long long y = (long long)curiosity.y + offset_y;
  if (x < 0 || x >= mars.largeur || y < 0 || y >= mars.hauteur) {
    return ROCK;
  }
  return mars.map[y][x];
}

int char_to_color(char c) {
  switch (c) {
  case TARGET:
    return CTARGET;
  case PLAIN:
    return CPLAIN;
  case MARK:
    return CMARK;
  case WATER:
    return CWATER;
  case ROCK:
    return CROCK;
  default:
    eprintf("Terrain inconnu: %c\n", c);
    assert(false);
  }
}

int char_to_mesure(char c) {
  switch (c) {
  case TARGET:
  case PLAIN:
    return 0;
  case MARK:
    return 1;
  case WATER:
    return 2;
  case ROCK:
    return 3;
  default:
    eprintf("Terrain inconnu: %c\n", c);
    assert(false);
  }
}

int mesure(int dir) { return char_to_mesure(charMesure(dir)); }

void pose(int arg) {
  if (arg == 0) {
    mars.map[curiosity.y][curiosity.x] = PLAIN;
  } else {
    mars.map[curiosity.y][curiosity.x] = MARK;
  }
}
