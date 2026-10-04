#!/bin/bash
CMAKE_DIRECTORY="build"
CLANG_TIDY="clang-tidy"
STD="c++26"

for var in "$@"; do
  echo "The flag \"$var\" is not recognized!" >&2
  exit 1
done

HEADER_FILTER="^$(pwd)/.*"

lint_files() {
  find . \( -path ./tests -prune -o -path "./$CMAKE_DIRECTORY" -prune -o -path ./build_debug -prune \) \
    -o -name "$1" -print0 \
    | xargs -0 -r -P"$(nproc)" -I{} "$CLANG_TIDY" -p "$CMAKE_DIRECTORY" \
        --extra-arg="-std=$STD" \
        --header-filter="$HEADER_FILTER" {} 2>&1 \
    | grep -E "warning:|error:|note:" | grep -v "^/usr"
}

output="$( { lint_files "*.hpp"; lint_files "*.cpp"; } )"

if [ -n "$output" ]; then
  echo "$output"
  if echo "$output" | grep -qE "warning:|error:"; then
    exit 1
  fi
fi
exit 0