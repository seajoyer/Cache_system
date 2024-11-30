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

          buildInputs = with pkgs; [ cmake gcc gtest direnv clang-tools bear ];

          CXX = "${pkgs.gcc}/bin/g++";
          CC = "${pkgs.gcc}/bin/gcc";

          shellHook = ''
            eval "$(direnv hook bash)"
            export CPATH=${pkgs.gtest.dev}/include:$CPATH
            echo "Environment is ready."
          '';
        };

        # Optional: If you use nix to build the project
        packages.default = pkgs.stdenv.mkDerivation {
          name = "Cache_system";
          src = ./.;

          buildInputs = [ pkgs.cmake pkgs.gcc pkgs.google-test ];

          buildPhase = ''
            mkdir -p build
            cd build
            cmake .. -DCMAKE_BUILD_TYPE=Release
            cmake --build .
          '';

          checkPhase = ''
            cd build
            ctest
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp -r build $out/bin/
          '';
        };
      });
}
