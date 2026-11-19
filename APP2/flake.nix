{
  description = "APP2: Curiosity interpreter (INF301)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "curiosity";
          version = "0.1.0";
          # Only what the build needs (not .venv, build/, bin/…)
          src = pkgs.lib.fileset.toSource {
            root = ./.;
            fileset = pkgs.lib.fileset.unions [
              ./Makefile
              ./src
              (pkgs.lib.fileset.maybeMissing ./tests)
            ];
          };

          enableParallelBuilding = true;
          buildFlags = [ "main" ];

          # Same optimisation as `make silent`, but with display kept on.
          # (passed through the environment: makeFlags would split on spaces)
          env = {
            CFLAGS = "-O3 -flto -Wall -Wextra -Werror -Wno-unused-parameter";
            LDFLAGS = "-O3 -flto";
          };

          # Run the functional tests when tests/ is present
          doCheck = builtins.pathExists ./tests;
          checkPhase = ''
            runHook preCheck
            for t in tests/*.test; do
              case "$t" in
                tests/labyrinthe.test|tests/surprise.test) continue ;; # AppoLab only
              esac
              ./main "$t" > /dev/null
            done
            runHook postCheck
          '';

          installPhase = ''
            runHook preInstall
            install -Dm755 main $out/bin/curiosity
            runHook postInstall
          '';

          meta.mainProgram = "curiosity";
        };

        gateau-fast = pkgs.pkgsStatic.stdenv.mkDerivation {
          pname = "gateau-fast";
          version = "0.1.0";
          # Credentials stay outside the build and the Nix store.
          src = pkgs.lib.fileset.toSource {
            root = ./.;
            fileset = pkgs.lib.fileset.unions [
              ./Makefile
              ./src
            ];
          };

          enableParallelBuilding = true;
          buildFlags = [ "bin/solve_gateau_fast" ];
          env = {
            CFLAGS = "-O2 -Wall -Wextra -Wno-unused-parameter";
            LDFLAGS = "-static";
          };

          installPhase = ''
            runHook preInstall
            install -Dm755 bin/solve_gateau_fast $out/bin/solve_gateau_fast
            runHook postInstall
          '';

          meta.mainProgram = "solve_gateau_fast";
        };
      });

      devShells = forAllSystems (pkgs: {
        portable = (pkgs.mkShell.override { stdenv = pkgs.pkgsStatic.stdenv; }) {
          packages = [ pkgs.gnumake ];
        };

        default = pkgs.mkShell {
          packages = with pkgs; [
            clang
            gnumake
            gdb
            valgrind # ./tests/check.py --mem c
            perf
            time # GNU time, used by perf.py
            (python3.withPackages (ps: [ ps.matplotlib ])) # perf.py graphs
          ];
        };
      });
    };
}
