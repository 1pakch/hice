with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "gcc-env";
  libraries = [
  ];
  buildInputs = with pkgs; [
    gcc
    gdb
    valgrind
    gnumake
    zlib
    clang_6
    #pkgs.pthreads
  ];

  # prevent clang package from interfering with options
  # https://github.com/NixOS/nixpkgs/issues/29877
  shellHook = ''
      export NIX_CXXSTDLIB_LINK=""
  '';
}
