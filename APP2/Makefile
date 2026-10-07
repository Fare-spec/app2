# I changed the makefile a bit so that all of my code can sit in src and be compiled in bin etc... It seemed better
.DEFAULT_GOAL := all

CC = clang
SRC_DIR := src
OBJ_DIR := build
PROGRAM := main
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

all: $(PROGRAM)

## Flags de compilation en mode debug, convient pour presque tout l'APP
CFLAGS ?= -g -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS += -MMD -MP
CPPFLAGS += -I$(SRC_DIR)

## Flags de compilation pour les tests de performance.
## A decommenter pour desactiver tous les affichages, puis forcer la 
## recompilation avec 'make -B'
## ou bien faire 'make silent' dans un terminal
# CFLAGS=-O3 -g -Wall -Wextra -Werror -DSILENT -Wno-unused-parameter -gdwarf-4



# Ici, on utilise l'"intelligence" de 'make' qui saura tout seul
# comment créer les .o à partir des .c
$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $@

-include $(DEPS)

silent:
	$(MAKE) clean
	$(MAKE) CPPFLAGS="$(CPPFLAGS) -DSILENT" CFLAGS="-O3 -g -Wall -Wextra -Werror -Wno-unused-parameter -MMD -MP" all


clean:
	rm -rf $(OBJ_DIR) $(PROGRAM)

.PHONY: all silent clean
