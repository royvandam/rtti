#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 --get-clangd-path | --get-cmake-path | --get-ninja-path | --get-cc-path | --get-cxx-path | --get-env-path" >&2
  exit 1
fi

case "$1" in
  --get-clangd-path)
    query='command -v clangd'
    ;;
  --get-cmake-path)
    query='command -v cmake'
    ;;
  --get-ninja-path)
    query='command -v ninja'
    ;;
  --get-cc-path)
    query='command -v "$CC"'
    ;;
  --get-cxx-path)
    query='command -v "$CXX"'
    ;;
  --get-env-path)
    query='printf "%s\n" "$PATH"'
    ;;
  *)
    echo "Unknown option: $1" >&2
    exit 1
    ;;
esac

workspace_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

# Always resolve this project's shell, even when called from another Nix shell.
exec nix-shell "$workspace_dir/shell.nix" --run "$query"
