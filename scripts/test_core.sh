#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
for capacity in 2 3 4 16 64; do
    ${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic -g -O1 \
      -fsanitize=address,undefined -fno-omit-frame-pointer \
      -DTT_CAPACITY=$capacity -Iinclude src/ttree_nodes.c src/ttree_search.c \
      src/ttree_insert.c src/ttree_balance.c tests/test_core.c -o "build/test_$capacity"
    "./build/test_$capacity"
done
