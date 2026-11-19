# APP2: Curiosity Interpreter

**Authors:** Kelvin Shammah, Elouan Fare

C interpreter for Curiosity, a stack-based language used to control a rover on Mars.

## Build and run

```sh
make
./main tests/simple.test
./main -d tests/if.test      # debug mode
```

## Tests

```sh
./tests/check.py c           # functional tests
./tests/check.py --mem c     # valgrind
make test                    # all tests
```

All tests pass without memory errors or leaks. `labyrinthe` and `surprise` are tested separately on AppoLab, since their local files are placeholders.

## Performance

```sh
make silent
./tests/performance/perf.py c
```

Time and memory usage are linear, except for `memfree`, which uses constant memory.

Results and analysis are in [`complexity.md`](complexity.md).

## Commands

| Command | Effect |
|---|---|
| `A` `D` `G` | Move forward, turn right/left |
| `0`–`9` | Push a number |
| `+` `-` `*` | Arithmetic |
| `M` | Measure in a given direction |
| `P` | Mark the current square (`0` clears it) |
| `{…}` | Push a routine |
| `!` | Execute a routine |
| `?` | Conditional execution |
| `B` | Repeat a routine |
| `C` `X` `I` | Duplicate, swap, drop |
| `R` | Rotate stack values |
| `Z` | Reverse the stack |

## Source files

| File | Purpose |
|---|---|
| `src/main.c` | Test loading and execution |
| `src/listes.c` | Program parsing and brace matching |
| `src/interprete.c` | Interpreter with iterative routine calls |
| `src/pile.c` | Dynamic stack |
| `src/curiosity.c`, `src/robot.c` | Map and rover |

The remaining files in `src/` are utilities for AppoLab challenges and are not used by `./main`, and for the `gateau` only the file solve_gateau_fast.c works ( it also uses regex for the pattern matching ).

## Portable gateau solver

Build a static musl executable with Nix:

```sh
nix build .#gateau-fast
file result/bin/solve_gateau_fast
```

Copy `result/bin/solve_gateau_fast` and your `.env` into a folder on the USB
stick, then run `./solve_gateau_fast` from that folder. Copy the executable,
not the `result` symlink. Credentials are read from `.env` at runtime and are
not bundled in the executable. The destination needs access to AppoLab and
the same CPU architecture as the build (x86-64 or ARM64). If the USB is mounted
with execution disabled, copy the files to a local folder and run them there.

For manual builds, `nix develop .#portable` provides the static musl toolchain:

```sh
make CC="$CC" OBJ_DIR=build-portable BIN_DIR=portable \
  CFLAGS="-O2 -Wall -Wextra -Wno-unused-parameter" LDFLAGS="-static" \
  portable/solve_gateau_fast
```
