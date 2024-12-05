{
  description = "Cache_system";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          name = "cache-system-devshell";

          buildInputs = with pkgs; [
            cmake
            gcc
            gtest
            catch
            direnv
            clang-tools
            bear
            nlohmann_json
            (python3.withPackages
              (ps: with ps; [ matplotlib psutil pybind11 numpy pybind11 ]))
          ];

          CXX = "${pkgs.gcc}/bin/g++";
          CC = "${pkgs.gcc}/bin/gcc";

          shellHook = ''
            export CCACHE_DIR=$HOME/.ccache
            export PATH="$HOME/.ccache/bin:$PATH"

            export CPATH=${pkgs.gtest.dev}/include:$CPATH
            export CPATH=${pkgs.catch}/include:$CPATH
            export CPATH=${pkgs.nlohmann_json}/include:$CPATH
            export CPATH=${pkgs.python3Packages.pybind11}/include:$CPATH
            echo "Environment is ready."
          '';
        };

        # Optional: If you use nix to build the project
        # packages.default = pkgs.stdenv.mkDerivation {
        #   name = "Cache_system";
        #   src = ./.;

        #   buildInputs = [ pkgs.cmake pkgs.gcc pkgs.google-test ];

        #   buildPhase = ''
        #     mkdir -p build
        #     cd build
        #     cmake .. -DCMAKE_BUILD_TYPE=Release
        #     cmake --build .
        #   '';

        #   checkPhase = ''
        #     cd build
        #     ctest
        #   '';

        #   installPhase = ''
        #     mkdir -p $out/bin
        #     cp -r build $out/bin/
        #   '';
        # };
      });
}
