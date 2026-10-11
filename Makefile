EXTENSION = ttree
MODULE_big = ttree
OBJS = src/ttree_nodes.o src/ttree_search.o src/ttree_insert.o src/ttree_balance.o src/pg_ttree.o
DATA = sql/ttree--1.0.sql
PG_CPPFLAGS = -I$(srcdir)/include
PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
