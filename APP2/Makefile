.DEFAULT_GOAL := all

CC ?= clang
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
CLIENT_DIR := $(SRC_DIR)/server-connection
MAKEFLAGS += -j$(shell nproc)

CFLAGS ?= -g -Wall -Wextra -Werror -Wno-unused-parameter 
CPPFLAGS += -I$(SRC_DIR)
DEPFLAGS := -MMD -MP

CORE_SOURCES := $(filter-out $(SRC_DIR)/main.c $(SRC_DIR)/solve_labyrinth.c $(SRC_DIR)/solving-fun.c $(SRC_DIR)/solve_gateau.c $(SRC_DIR)/solve_gateau_retry.c $(SRC_DIR)/solve_gateau_flow.c $(SRC_DIR)/solve_gateau_fast.c $(SRC_DIR)/solve_gateau_experiment.c $(SRC_DIR)/analyze_gateau.c $(SRC_DIR)/probe_gateau.c $(SRC_DIR)/list_gateau_candidates.c $(SRC_DIR)/replay_gateau.c,$(wildcard $(SRC_DIR)/*.c))
CORE_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SOURCES))
CLIENT_OBJECTS := $(OBJ_DIR)/server-client.o $(OBJ_DIR)/server-client-crypteMove.o
APPOLAB_OBJECT := $(OBJ_DIR)/appolab.o
MAIN_OBJECTS := $(OBJ_DIR)/main.o $(CORE_OBJECTS)
SOLVER_OBJECTS := $(OBJ_DIR)/solve_labyrinth.o $(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
GATEAU_OBJECTS := $(OBJ_DIR)/solve_gateau.o $(OBJ_DIR)/gateau-index.o \
	$(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
ANALYZE_GATEAU_OBJECTS := $(OBJ_DIR)/analyze_gateau.o $(OBJ_DIR)/gateau-protocol.o \
	$(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
GATEAU_FLOW_OBJECTS := $(OBJ_DIR)/solve_gateau_flow.o $(OBJ_DIR)/gateau-flow.o \
	$(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
GATEAU_FAST_OBJECTS := $(OBJ_DIR)/solve_gateau_fast.o $(OBJ_DIR)/gateau-flow.o \
	$(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
GATEAU_EXPERIMENT_OBJECTS := $(OBJ_DIR)/solve_gateau_experiment.o \
	$(OBJ_DIR)/gateau-flow.o $(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
PROBE_GATEAU_OBJECTS := $(OBJ_DIR)/probe_gateau.o $(OBJ_DIR)/gateau-probe.o \
	$(CLIENT_OBJECTS) $(APPOLAB_OBJECT)
CANDIDATE_OBJECTS := $(OBJ_DIR)/gateau-candidates.o
LIST_CANDIDATES_OBJECTS := $(OBJ_DIR)/list-gateau-candidates.o $(CANDIDATE_OBJECTS)
REPLAY_GATEAU_OBJECTS := $(OBJ_DIR)/replay_gateau.o $(OBJ_DIR)/gateau-replay.o $(CANDIDATE_OBJECTS)
GATEAU_RETRY_OBJECTS := $(OBJ_DIR)/solve_gateau_retry.o \
	$(OBJ_DIR)/gateau-index.o $(CLIENT_OBJECTS) $(APPOLAB_OBJECT)

CORE_TESTS := simple calculs mesures marques marques-et-mesures copie \
	if if-non-depile empile imbrique exec echanges clones compteur boucle \
	boucle-sans-pile rotations ifignore boucle-if boucle-clone langton \
	mysterieuZe
LABYRINTH_CANDIDATE := {{{{3M2-{DA}{G}?}9B}9B}9B}9B

all: main $(BIN_DIR)/main $(BIN_DIR)/solve_labyrinth $(BIN_DIR)/solving-fun \
	$(BIN_DIR)/solve_gateau $(BIN_DIR)/solve_gateau_retry \
	$(BIN_DIR)/analyze_gateau $(BIN_DIR)/solve_gateau_flow \
	$(BIN_DIR)/solve_gateau_fast $(BIN_DIR)/probe_gateau \
	$(BIN_DIR)/list_gateau_candidates $(BIN_DIR)/replay_gateau \
	$(BIN_DIR)/solve_gateau_experiment

$(BIN_DIR)/main: $(MAIN_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# tests/check.py and perf.py run ./main from this directory
main: $(BIN_DIR)/main
	cp $< $@

$(BIN_DIR)/solve_labyrinth: $(SOLVER_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solving-fun: $(OBJ_DIR)/solving-fun.o $(CORE_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solve_gateau: $(GATEAU_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solve_gateau_retry: $(GATEAU_RETRY_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/analyze_gateau: $(ANALYZE_GATEAU_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solve_gateau_flow: $(GATEAU_FLOW_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solve_gateau_fast: $(GATEAU_FAST_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/probe_gateau: $(PROBE_GATEAU_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/list_gateau_candidates: $(LIST_CANDIDATES_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/replay_gateau: $(REPLAY_GATEAU_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR)/solve_gateau_experiment: $(GATEAU_EXPERIMENT_OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_DIR)/solve_labyrinth.o: $(SRC_DIR)/solve_labyrinth.c $(CLIENT_DIR)/client.h $(CLIENT_DIR)/appolab.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -c -o $@ $<

$(OBJ_DIR)/solve_gateau.o: $(SRC_DIR)/solve_gateau.c $(CLIENT_DIR)/client.h $(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/index.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/solve_gateau_retry.o: $(SRC_DIR)/solve_gateau_retry.c $(CLIENT_DIR)/client.h $(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/index.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/gateau-index.o: $(SRC_DIR)/gateau/index.c $(SRC_DIR)/gateau/index.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/gateau-protocol.o: $(SRC_DIR)/gateau/protocol.c $(SRC_DIR)/gateau/protocol.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/gateau-flow.o: $(SRC_DIR)/gateau/flow.c $(SRC_DIR)/gateau/flow.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/solve_gateau_flow.o: $(SRC_DIR)/solve_gateau_flow.c $(CLIENT_DIR)/client.h \
	$(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/flow.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/solve_gateau_fast.o: $(SRC_DIR)/solve_gateau_fast.c $(CLIENT_DIR)/client.h \
	$(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/flow.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/solve_gateau_experiment.o: $(SRC_DIR)/solve_gateau_experiment.c \
	$(CLIENT_DIR)/client.h $(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/flow.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/gateau-probe.o: $(SRC_DIR)/gateau/probe.c $(SRC_DIR)/gateau/probe.h \
	$(CLIENT_DIR)/client.h $(CLIENT_DIR)/appolab.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/probe_gateau.o: $(SRC_DIR)/probe_gateau.c $(SRC_DIR)/gateau/probe.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR) -c -o $@ $<

$(OBJ_DIR)/gateau-candidates.o: $(SRC_DIR)/gateau/candidates.c $(SRC_DIR)/gateau/candidates.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/list-gateau-candidates.o: $(SRC_DIR)/list_gateau_candidates.c $(SRC_DIR)/gateau/candidates.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR) -c -o $@ $<

$(OBJ_DIR)/gateau-replay.o: $(SRC_DIR)/gateau/replay.c $(SRC_DIR)/gateau/replay.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/replay_gateau.o: $(SRC_DIR)/replay_gateau.c $(SRC_DIR)/gateau/candidates.h $(SRC_DIR)/gateau/replay.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I$(SRC_DIR) -c -o $@ $<

$(OBJ_DIR)/analyze_gateau.o: $(SRC_DIR)/analyze_gateau.c $(CLIENT_DIR)/client.h \
	$(CLIENT_DIR)/appolab.h $(SRC_DIR)/gateau/protocol.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/gateau -c -o $@ $<

$(OBJ_DIR)/appolab.o: $(CLIENT_DIR)/appolab.c $(CLIENT_DIR)/appolab.h $(CLIENT_DIR)/client.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -c -o $@ $<

$(OBJ_DIR)/solving-fun.o: $(SRC_DIR)/solving-fun.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_DIR)/server-client.o: $(CLIENT_DIR)/client.c $(CLIENT_DIR)/client.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Wno-unused-result -I$(CLIENT_DIR) -c -o $@ $<

$(OBJ_DIR)/server-client-crypteMove.o: $(CLIENT_DIR)/client-crypteMove.c $(CLIENT_DIR)/client-crypteMove.h $(CLIENT_DIR)/client.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(CLIENT_DIR) -c -o $@ $<

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

test: all
	@set -e; for name in $(CORE_TESTS); do \
		./$(BIN_DIR)/main tests/$$name.test >/dev/null; \
		printf 'PASS %s\n' $$name; \
	done
	@./$(BIN_DIR)/solving-fun tests/labyrinthe.test tests/surprise.test >/dev/null
	@printf 'PASS solving-fun local maze synthesis\n'
	@./$(BIN_DIR)/solving-fun --check-program '$(LABYRINTH_CANDIDATE)' \
		tests/labyrinthe.test tests/surprise.test
	@./$(BIN_DIR)/solving-fun tests/labyrinthe.test tests/surprise.test \
		> $(OBJ_DIR)/synthesis.out
	@set -e; candidate="$$(tail -n 1 $(OBJ_DIR)/synthesis.out)"; \
	awk -v solver="$$candidate" 'BEGIN { done = 0 } \
		!done && $$0 == "AAAAAAAAAAAAAAAAA" { print solver; done = 1; next } \
		{ print } END { if (!done) exit 1 }' tests/labyrinthe.test \
		> $(OBJ_DIR)/cegis-labyrinthe.test; \
	./$(BIN_DIR)/main $(OBJ_DIR)/cegis-labyrinthe.test >/dev/null; \
	printf 'PASS CEGIS-generated routine on supplied maze\n'
	@set -e; solver="$$($(BIN_DIR)/solve_labyrinth --program)"; \
	awk -v solver="$$solver" 'BEGIN { done = 0 } \
		!done && $$0 == "AAAAAAAAAAAAAAAAA" { print solver; done = 1; next } \
		{ print } END { if (!done) exit 1 }' tests/labyrinthe.test \
		> $(OBJ_DIR)/labyrinthe-solver.test; \
	./$(BIN_DIR)/main $(OBJ_DIR)/labyrinthe-solver.test >/dev/null; \
	printf 'PASS generated solver on supplied maze\n'

silent:
	$(MAKE) clear
	$(MAKE) CFLAGS="-O3 -flto -g -Wall -Wextra -Werror -Wno-unused-parameter -DSILENT" LDFLAGS="-O3 -flto" main

clean:
	rm -rf $(OBJ_DIR)

clear:
	rm -rf $(OBJ_DIR) $(BIN_DIR) main

-include $(wildcard $(OBJ_DIR)/*.d)

.PHONY: all test silent clean clear
