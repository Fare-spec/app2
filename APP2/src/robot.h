#ifndef ROBOT_H
#define ROBOT_H

typedef enum { Nord, Est, Sud, Ouest } Orientation;

typedef struct {
  int x;
  int y;
  Orientation o;
} Robot;

void init_robot(Robot *robot, int x, int y, Orientation orientation);
void avancer(Robot *robot);
void tourner_a_gauche(Robot *robot);
void tourner_a_droite(Robot *robot);
void position(Robot *robot, int *x, int *y);
int abscisse(Robot *robot);
int ordonnee(Robot *robot);
Orientation orient(Robot *robot);
void position_devant(Robot *robot, int *x, int *y);

#endif
