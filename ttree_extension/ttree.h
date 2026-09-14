#ifndef TTREE_H
#define TTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define NODE_CAPACITY 8 // Número máximo de elementos por nodo

typedef int KeyType; // Tipo de clave base

// Estructura del nodo individual del T-Tree
typedef struct TTreeNode {
    KeyType keys[NODE_CAPACITY];
    int count;                  // Cantidad de elementos ocupados (0 a NODE_CAPACITY)
    int height;                 // Requerido por I4 para calcular el factor de balanceo
    struct TTreeNode *left;
    struct TTreeNode *right;
} TTreeNode;

// Estructura contenedora del árbol
typedef struct TTree {
    TTreeNode *root;
    size_t total_items;
} TTree;

/* Módulo Integrante 1: Gestión de memoria y estructuras base */
TTreeNode* ttree_create_node(void);
void ttree_free_node(TTreeNode *node);
TTree* ttree_create(void);
void ttree_destroy(TTree *tree);
bool ttree_build(TTree *tree, const KeyType *keys, size_t count);

/* Módulo Integrante 3: Firma requerida para ttree_build */
bool ttree_insert(TTree *tree, KeyType key);

#endif
