{ pkgs, stdenv, ncurses }:
stdenv.mkDerivation {
  name = "term-tetris";
  version = "1.0.0";
  description = "Modern tetris-like written in ncurses with simple multiplayer";

  src = ./.;

  buildInputs = [ ncurses ];

  buildPhase = ''
    runHook preBuild
    make
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    cp tetris $out/bin
    cp server $out/bin/tetris-server
    runHook postInstall
  '';
}
