#include "robot.h"

void init_robot(Robot *robot, int x, int y, Orientation orientation) {
  robot->x = x;
  robot->y = y;
  robot->o = orientation;
}

void avancer(Robot *robot) {
  robot->x += (robot->o & 1) * (2 - robot->o);
  robot->y -= ((robot->o + 1) & 1) * (1 - robot->o);
}

void tourner_a_gauche(Robot *robot) { robot->o = (robot->o + 3) & 3; }

void tourner_a_droite(Robot *robot) { robot->o = (robot->o + 1) & 3; }

void position(Robot *robot, int *x, int *y) {
  *x = robot->x;
  *y = robot->y;
}

int abscisse(Robot *robot) { return robot->x; }

int ordonnee(Robot *robot) { return robot->y; }

Orientation orient(Robot *robot) { return robot->o; }

void position_devant(Robot *robot, int *x, int *y) {
  *x = robot->x + (robot->o & 1) * (2 - robot->o);
  *y = robot->y - ((robot->o + 1) & 1) * (1 - robot->o);
}
