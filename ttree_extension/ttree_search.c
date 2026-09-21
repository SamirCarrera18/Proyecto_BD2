#include "ttree.h"

bool ttree_search(const TTree *tree, KeyType key) {
    if (tree == NULL) {
        return false;
    }

    const TTreeNode *current = tree->root;

    while (current != NULL) {

        // Caso 1: la clave es menor que la primera del nodo
        if (key < current->keys[0]) {
            current = current->left;
        }

        // Caso 2: la clave es mayor que la última del nodo
        else if (key > current->keys[current->count - 1]) {
            current = current->right;
        }

        // Caso 3: la clave está dentro del rango del nodo
        else {
            for (int i = 0; i < current->count; i++) {
                if(current->keys[i] == key) {
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}