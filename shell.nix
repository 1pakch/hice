with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "gcc-env";
  libraries = [
  ];
  buildInputs = [
    pkgs.gcc
    pkgs.gdb
    pkgs.valgrind
    pkgs.gnumake
    pkgs.zlib
    pkgs.clang_6
    #pkgs.pthreads
  ];
}
