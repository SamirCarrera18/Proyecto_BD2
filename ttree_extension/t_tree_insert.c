#include "ttree.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Provisorio: Declaración externa o stub de la función de balanceo/división del Integrante 4 */
extern void balance_after_insert(TTreeNode **root, TTreeNode *inserted_node);

/* Función auxiliar para insertar una clave y su TID de forma ordenada en un nodo hoja */
bool insert_into_node_sorted(TTreeNode *node, int key, ItemPointer tid) {
    if (node->count >= MAX_ITEMS) {
        return false; // El nodo está lleno
    }

    int i = node->count - 1;
    // Desplazar elementos mayores hacia la derecha para mantener el orden
    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        node->tids[i + 1] = node->tids[i];
        i--;
    }

    // Insertar en la posición correcta
    node->keys[i + 1] = key;
    node->tids[i + 1] = tid;
    node->count++;
    return true;
}

/* Función principal de Inserción (Integrante 3) */
bool t_tree_insert(TTreeNode **root, int key, ItemPointer tid) {
    if (*root == nullptr) {
        // Si el árbol está vacío, creamos el nodo raíz inicial
        *root = (TTreeNode *)palloc0(sizeof(TTreeNode));
        (*root)->keys[0] = key;
        (*root)->tids[0] = tid;
        (*root)->count = 1;
        (*root)->balance_factor = 0;
        (*root)->left = NULL;
        (*root)->right = NULL;
        (*root)->parent = NULL;
        return true;
    }

    TTreeNode *current = *root;
    TTreeNode *parent = NULL;
    bool go_left = false;

    // 1. Ubicar el nodo hoja o el lugar donde corresponda insertar (siguiendo lógica de árbol binario/T-Tree)
    while (current != NULL) {
        parent = current;
        // Asumiendo política de rangos del T-Tree: si es menor que el menor, va a la izquierda; si es mayor que el mayor, a la derecha. 
        // Si cae dentro del rango del nodo, se intenta insertar aquí.
        if (key < current->keys[0]) {
            current = current->left;
            go_left = true;
        } else if (key > current->keys[current->count - 1]) {
            current = current->right;
            go_left = false;
        } else {
            // La clave pertenece a este nodo (está dentro de sus límites o duplicada permitida)
            break;
        }
    }

    // Si salimos del ciclo y 'current' es NULL, debemos insertarlo en un nuevo hijo del último 'parent' recorrido
    if (current == NULL) {
        current = (TTreeNode *)palloc0(sizeof(TTreeNode));
        current->keys[0] = key;
        current->tids[0] = tid;
        current->count = 1;
        current->parent = parent;
        
        if (go_left) {
            parent->left = current;
        } else {
            parent->right = current;
        }
        
        // Invocar balanceo del Integrante 4
        balance_after_insert(root, current);
        return true;
    }

    // 2. Insertar ordenadamente dentro del nodo encontrado
    if (current->count < MAX_ITEMS) {
        return insert_into_node_sorted(current, key, tid);
    } 
    
    // 3. Manejar nodos llenos conservando las claves existentes (División / Split)
    else {
        // Creamos un nodo hermano derecho para dividir el exceso de elementos
        TTreeNode *new_node = (TTreeNode *)palloc0(sizeof(TTreeNode));
        
        // Copiamos temporalmente las claves a un arreglo auxiliar junto con la nueva clave
        int temp_keys[MAX_ITEMS + 1];
        ItemPointerData temp_tids[MAX_ITEMS + 1];
        
        int idx = 0;
        bool inserted = false;
        
        for (int i = 0; i < current->count; i++) {
            if (!inserted && key < current->keys[i]) {
                temp_keys[idx] = key;
                temp_tids[idx] = tid;
                idx++;
                inserted = true;
            }
            temp_keys[idx] = current->keys[i];
            temp_tids[idx] = current->tids[i];
            idx++;
        }
        if (!inserted) {
            temp_keys[idx] = key;
            temp_tids[idx] = tid;
        }

        // Repartir equitativamente los elementos entre el nodo original y el nuevo nodo
        int total = MAX_ITEMS + 1;
        int mid = total / 2;

        current->count = 0;
        new_node->count = 0;
        new_node->parent = current->parent;

        // Llenar nodo izquierdo (current)
        for (int i = 0; i < mid; i++) {
            current->keys[i] = temp_keys[i];
            current->tids[i] = temp_tids[i];
            current->count++;
        }

        // Llenar nodo derecho (new_node)
        for (int i = mid; i < total; i++) {
            new_node->keys[new_node->count] = temp_keys[i];
            new_node->tids[new_node->count] = temp_tids[i];
            new_node->count++;
        }

        // Conectar el nuevo nodo al padre (coordinación con Integrante 4 para rebalanceo estructural)
        // Por simplicidad de la interfaz, delegamos la inserción del nodo dividido al balanceador
        balance_after_insert(root, new_node);
        
        return true;
    }
}