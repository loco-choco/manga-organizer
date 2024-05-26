{ pkgs ? import <nixpkgs> { } }:
with pkgs;
mkShell {
  buildInputs = [
    stdenv
    ncurses
  ];

  shellHook = ''
    # ...
  '';
}
