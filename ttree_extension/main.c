#include <stdio.h>
#include "ttree.h"

// Función mock (simulada) para emular el trabajo del Integrante 3
bool ttree_insert(TTree *tree, KeyType key) {
    if (!tree) return false;
    
    if (tree->root == NULL) {
        tree->root = ttree_create_node();
        if (!tree->root) return false;
    }
    
    printf("Insertando clave simulada: %d\n", key);
    tree->total_items++;
    return true;
}

int main(void) {
    TTree *tree = ttree_create();
    if (!tree) {
        printf("Error al crear la estructura del árbol.\n");
        return 1;
    }

    KeyType datos[] = {10, 20, 5, 15, 30};
    size_t n = sizeof(datos) / sizeof(datos[0]);

    printf("--- Probando Construcción en C (Integrante 1) ---\n");
    if (ttree_build(tree, datos, n)) {
        printf("Árbol construido correctamente. Elementos totales: %zu\n", tree->total_items);
    }

    ttree_destroy(tree);
    printf("--- Memoria liberada correctamente con ttree_destroy() ---\n");

    return 0;
}
