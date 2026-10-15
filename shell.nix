{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  packages = with pkgs; [
    cmake
    ninja
    clang-tools
    cppcheck
    gdb
    git
  ];

  CMAKE_GENERATOR = "Ninja";
  CMAKE_EXPORT_COMPILE_COMMANDS = "ON";
}
