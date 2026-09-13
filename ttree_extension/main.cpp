#include <iostream>
#include "ttree.hpp"

// Simulación temporal del trabajo del Integrante 3
bool TTree::insert(KeyType key) {
    // Si la raíz no existe, creamos un nodo inicial simulado
    if (root == nullptr) {
        root = new TTreeNode();
    }
    
    std::cout << "Insertando clave simulada: " << key << std::endl;
    total_items++;
    return true; 
}

int main() {
    TTree tree;
    std::vector<KeyType> datos = {10, 20, 5, 15, 30};

    std::cout << "--- Probando Construcción del Integrante 1 ---" << std::endl;
    if (tree.build(datos)) {
        std::cout << "Árbol construido correctamente. Elementos totales: " 
                  << tree.getTotalItems() << std::endl;
    }

    std::cout << "--- Fin del programa (El destructor ~TTree() liberará la memoria automáticamente) ---" << std::endl;
    return 0;
}