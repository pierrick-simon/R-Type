#!/bin/bash
CMAKE_DIRECTORY="build"

# Internal variables. Do not modify!
debug=true

for var in "$@"; do
  if [ $var == "--fix" ]; then
      debug=false;
  else
    echo "The flag \"$var\" is not recognized!" >&2;
    exit 1;
  fi
done
exec_cmd=""
if [ $debug == true ]; then
    exec_cmd="clang-format-22 -style=file --dry-run {}"
else
    exec_cmd="clang-format-22 -style=file -i {}"
fi

find . \( -name tests -prune -o -name $CMAKE_DIRECTORY -prune \) -o -name *.cpp -exec $exec_cmd \;
find . \( -name tests -prune -o -name $CMAKE_DIRECTORY -prune \) -o -name *.hpp -exec $exec_cmd \;