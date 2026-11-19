#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curiosity.h"
#include "interprete.h"
#include "listes.h"

#define MAX_MAZES 64
#define MAX_SIDE 255
#define LOOP_DEPTHS 6
#define MAX_CANDIDATES (4 * 2 * LOOP_DEPTHS)

typedef struct {
  char name[256];
  int width;
  int height;
  int start_x;
  int start_y;
  int goal_x;
  int goal_y;
  char cells[MAX_SIDE][MAX_SIDE];
} maze_t;

typedef struct {
  int check_turn;
  int blocked_turn;
  int loop_depth;
  char program[512];
} candidate_t;

static maze_t mazes[MAX_MAZES];
static candidate_t candidates[MAX_CANDIDATES];
static size_t candidate_count;
static const int check_turns[] = {1, 3, 0, 2};
static const int measurement_direction[] = {1, 3, 5, 7};
static const char *open_action[] = {"A", "DA", "DDA", "GA"};
static const int blocked_turns[] = {3, 1};

static bool is_map_character(char value) {
  return value == '~' || value == '#' || value == '.' || value == 'C' ||
         value == 'P' || value == 'M' || value == '@';
}

static bool load_maze(const char *filename, maze_t *maze) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror(filename);
    return false;
  }

  memset(maze, 0, sizeof *maze);
  snprintf(maze->name, sizeof maze->name, "%s", filename);
  char *line = NULL;
  size_t capacity = 0;
  bool in_map = false;
  bool valid = true;

  while (getline(&line, &capacity, file) >= 0) {
    size_t length = strcspn(line, "\r\n");
    line[length] = '\0';
    if (strncmp(line, "Map", 3) == 0) {
      in_map = true;
      continue;
    }
    if (!in_map) {
      continue;
    }
    if (length == 0) {
      if (maze->height > 0) {
        break;
      }
      continue;
    }

    bool row = length <= MAX_SIDE;
    for (size_t i = 0; row && i < length; i++) {
      row = is_map_character(line[i]);
    }
    if (!row) {
      if (maze->height > 0) {
        break;
      }
      continue;
    }
    if (maze->width == 0) {
      maze->width = (int)length;
    } else if (maze->width != (int)length) {
      fprintf(stderr, "%s: maze rows have different widths\n", filename);
      valid = false;
      break;
    }
    if (maze->height >= MAX_SIDE) {
      fprintf(stderr, "%s: maze is too tall\n", filename);
      valid = false;
      break;
    }

    int y = maze->height++;
    memcpy(maze->cells[y], line, length);
    for (int x = 0; x < maze->width; x++) {
      if (line[x] == 'C' || line[x] == 'P') {
        maze->start_x = x;
        maze->start_y = y;
      } else if (line[x] == '@') {
        maze->goal_x = x;
        maze->goal_y = y;
      }
    }
  }

  free(line);
  fclose(file);
  if (valid && (maze->width == 0 || maze->height == 0)) {
    fprintf(stderr, "%s: no map found\n", filename);
    valid = false;
  }
  if (valid && maze->cells[maze->start_y][maze->start_x] != 'C' &&
      maze->cells[maze->start_y][maze->start_x] != 'P') {
    fprintf(stderr, "%s: no Curiosity start cell found\n", filename);
    valid = false;
  }
  if (valid && maze->cells[maze->goal_y][maze->goal_x] != '@') {
    fprintf(stderr, "%s: no target found\n", filename);
    valid = false;
  }
  return valid;
}

static bool traversable(const maze_t *maze, int x, int y) {
  if (x < 0 || x >= maze->width || y < 0 || y >= maze->height) {
    return false;
  }
  char cell = maze->cells[y][x];
  return cell != '~' && cell != '#';
}

static bool candidate_exits(const maze_t *maze, const candidate_t *candidate) {
  static const int dx[] = {1, 0, -1, 0};
  static const int dy[] = {0, 1, 0, -1};
  size_t state_count = (size_t)maze->width * (size_t)maze->height * 4;
  bool *visited = calloc(state_count, sizeof *visited);
  if (visited == NULL) {
    return false;
  }

  int x = maze->start_x;
  int y = maze->start_y;
  int direction = 0;
  int step_limit = 1;
  for (int i = 0; i < candidate->loop_depth; i++) {
    step_limit *= 9;
  }

  bool exits = false;
  for (int step = 0; step < step_limit; step++) {
    size_t state = ((size_t)y * (size_t)maze->width + (size_t)x) * 4 +
                   (size_t)direction;
    if (visited[state]) {
      break;
    }
    visited[state] = true;

    int next_direction = (direction + candidate->check_turn) % 4;
    int next_x = x + dx[next_direction];
    int next_y = y + dy[next_direction];
    bool is_water = next_x >= 0 && next_x < maze->width && next_y >= 0 &&
                    next_y < maze->height && maze->cells[next_y][next_x] == '~';
    if (is_water) {
      direction = (direction + candidate->blocked_turn) % 4;
      continue;
    }
    if (!traversable(maze, next_x, next_y)) {
      break;
    }
    x = next_x;
    y = next_y;
    direction = next_direction;
    if (x == maze->goal_x && y == maze->goal_y) {
      exits = true;
      break;
    }
  }

  free(visited);
  return exits;
}

static bool program_exits(const maze_t *maze, const char *program) {
  size_t program_length = strlen(program);
  char *mutable_program = malloc(program_length + 1);
  if (mutable_program == NULL) {
    return false;
  }
  memcpy(mutable_program, program, program_length + 1);

  initCarte(1);
  for (int y = 0; y < maze->height; y++) {
    char row[MAX_SIDE + 2];
    memcpy(row, maze->cells[y], (size_t)maze->width);
    row[maze->width] = '\n';
    row[maze->width + 1] = '\0';
    ajoutLigneCarte(row);
  }

  sequence_t sequence = {0};
  conversion(mutable_program, &sequence);
  free(mutable_program);
  silent_mode = true;
  int result = interprete(&sequence, false);
  bool exits = result == VICTOIRE && curiosity.x >= 0 &&
               curiosity.y >= 0 &&
               mars.map[curiosity.y][curiosity.x] == TARGET;
  clear_sequence(&sequence);
  return exits;
}

static int check_program(const char *program, int argc, char **argv) {
  size_t maze_count = (size_t)(argc - 3);
  if (maze_count == 0 || maze_count > MAX_MAZES) {
    fprintf(stderr, "Provide between 1 and %d maze files\n", MAX_MAZES);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < maze_count; i++) {
    if (!load_maze(argv[i + 3], &mazes[i])) {
      return EXIT_FAILURE;
    }
  }

  bool all_pass = true;
  for (size_t i = 0; i < maze_count; i++) {
    if (program_exits(&mazes[i], program)) {
      printf("PASS %s\n", mazes[i].name);
    } else {
      printf("Counterexample: %s\n", mazes[i].name);
      all_pass = false;
    }
  }
  return all_pass ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool passes_examples(const candidate_t *candidate,
                            const bool *examples, size_t maze_count) {
  for (size_t i = 0; i < maze_count; i++) {
    if (examples[i] && !candidate_exits(&mazes[i], candidate)) {
      return false;
    }
  }
  return true;
}

static bool build_program(candidate_t *candidate) {
  char body[384];
  const char *blocked_action = candidate->blocked_turn == 1 ? "D" : "G";
  snprintf(body, sizeof body, "%dM2-{%s}{%s}?",
           measurement_direction[candidate->check_turn],
           open_action[candidate->check_turn], blocked_action);

  for (int i = 0; i < candidate->loop_depth; i++) {
    char next[384];
    int written = snprintf(next, sizeof next, "{%s}9B", body);
    if (written < 0 || (size_t)written >= sizeof next) {
      return false;
    }
    memcpy(body, next, (size_t)written + 1);
  }
  snprintf(candidate->program, sizeof candidate->program, "%s", body);
  return true;
}

static void save_candidate(int check_turn, int blocked_turn, int loop_depth) {
  if (candidate_count >= MAX_CANDIDATES) {
    return;
  }
  candidate_t *candidate = &candidates[candidate_count];
  candidate->check_turn = check_turn;
  candidate->blocked_turn = blocked_turn;
  candidate->loop_depth = loop_depth;
  if (build_program(candidate)) {
    candidate_count++;
  }
}

static int synthesize(const bool *examples, size_t maze_count) {
  int best = -1;
  for (size_t i = 0; i < candidate_count; i++) {
    if (!passes_examples(&candidates[i], examples, maze_count)) {
      continue;
    }
    if (best < 0 || strlen(candidates[i].program) <
                        strlen(candidates[best].program)) {
      best = (int)i;
    }
  }
  return best;
}

int main(int argc, char **argv) {
  if (argc >= 2 && strcmp(argv[1], "--check-program") == 0) {
    if (argc < 4) {
      fprintf(stderr,
              "Usage: %s --check-program '<Curiosity code>' maze.test [...]\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return check_program(argv[2], argc, argv);
  }

  const char *default_maze = "tests/labyrinthe.test";
  size_t maze_count = argc == 1 ? 1 : (size_t)(argc - 1);
  if (maze_count > MAX_MAZES) {
    fprintf(stderr, "At most %d maze files can be used\n", MAX_MAZES);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < maze_count; i++) {
    const char *filename = argc == 1 ? default_maze : argv[i + 1];
    if (!load_maze(filename, &mazes[i])) {
      return EXIT_FAILURE;
    }
  }

  for (size_t i = 0; i < sizeof check_turns / sizeof check_turns[0]; i++) {
    for (size_t j = 0; j < sizeof blocked_turns / sizeof blocked_turns[0]; j++) {
      for (int depth = 1; depth <= LOOP_DEPTHS; depth++) {
        save_candidate(check_turns[i], blocked_turns[j], depth);
      }
    }
  }

  bool examples[MAX_MAZES] = {false};
  examples[0] = true;
  size_t counterexample_count = 0;
  size_t rounds = 0;

  while (true) {
    rounds++;
    int candidate = synthesize(examples, maze_count);
    if (candidate < 0) {
      fprintf(stderr,
              "No candidate in the enumerated grammar passes the current "
              "examples\n");
      return EXIT_FAILURE;
    }

    size_t counterexample = maze_count;
    for (size_t i = 0; i < maze_count; i++) {
      if (!examples[i] && !candidate_exits(&mazes[i], &candidates[candidate])) {
        counterexample = i;
        break;
      }
    }
    if (counterexample == maze_count) {
      printf("Candidates enumerated: %zu one-sensor policy/loop combinations\n",
             candidate_count);
      printf("CEGIS rounds: %zu\n", rounds);
      printf("Counterexamples added: %zu\n", counterexample_count);
      printf("Shortest passing routine in this grammar: %zu characters\n",
             strlen(candidates[candidate].program));
      printf("Loop depth: %d\n", candidates[candidate].loop_depth);
      printf("%s\n", candidates[candidate].program);
      return EXIT_SUCCESS;
    }

    examples[counterexample] = true;
    counterexample_count++;
    printf("Counterexample added: %s\n", mazes[counterexample].name);
  }
}
