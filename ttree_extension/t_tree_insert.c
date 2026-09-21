#include "ttree.h"

/* Función auxiliar para insertar una clave ordenadamente en el arreglo del nodo */
static bool insert_into_node_sorted(TTreeNode *node, KeyType key) {
    if (node->count >= NODE_CAPACITY) {
        return false; // El nodo está lleno
    }

    int i = node->count - 1;
    // Desplazar elementos mayores a la derecha para mantener el orden
    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        i--;
    }

    // Insertar en la posición correcta
    node->keys[i + 1] = key;
    node->count++;
    return true;
}

/* Módulo Integrante 3: Inserción adaptada a la estructura base de I1 */
bool ttree_insert(TTree *tree, KeyType key) {
    if (!tree) {
        return false;
    }

    // Si el árbol está vacío, creamos el nodo raíz inicial
    if (tree->root == NULL) {
        tree->root = ttree_create_node();
        if (!tree->root) {
            return false;
        }
        tree->root->keys[0] = key;
        tree->root->count = 1;
        tree->total_items++;
        return true;
    }

    TTreeNode *current = tree->root;

    // Recorrido para ubicar el nodo o crear un nuevo hijo
    while (current != NULL) {
        // Si la clave es menor que el menor elemento del nodo actual
        if (key < current->keys[0]) {
            if (current->left == NULL) {
                current->left = ttree_create_node();
                if (!current->left) return false;
                current->left->keys[0] = key;
                current->left->count = 1;
                tree->total_items++;
                return true;
            }
            current = current->left;
        }
        // Si la clave es mayor que el mayor elemento del nodo actual
        else if (key > current->keys[current->count - 1]) {
            if (current->right == NULL) {
                current->right = ttree_create_node();
                if (!current->right) return false;
                current->right->keys[0] = key;
                current->right->count = 1;
                tree->total_items++;
                return true;
            }
            current = current->right;
        }
        // Si la clave cae dentro del rango del nodo actual
        else {
            if (current->count < NODE_CAPACITY) {
                bool inserted = insert_into_node_sorted(current, key);
                if (inserted) {
                    tree->total_items++;
                }
                return inserted;
            } else {
                // Nodo lleno: aquí se integrará la lógica de división y balanceo (I4)
                return false;
            }
        }
    }

    return false;
}