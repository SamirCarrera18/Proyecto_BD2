#ifndef TTREE_H
#define TTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef TT_CAPACITY
#define TT_CAPACITY 16
#endif

#if TT_CAPACITY < 2
#error "TT_CAPACITY debe ser >= 2"
#endif

#define TT_MIN_INTERNAL (TT_CAPACITY - 1)//Esto significa que un nodo interno 
//con dos hijos debe mantener por lo menos 15 claves.

typedef void *(*TTAlloc)(size_t, void *);//representa un puntero a una función
typedef void (*TTFree)(void *, void *);//Función para liberar memoria

typedef struct TTNode {
    int32_t keys[TT_CAPACITY];
    size_t count;
    int height;
    struct TTNode *left, *right;
} TTNode;

typedef struct TTree {
    TTNode *root;
    size_t size, nodes;
    TTAlloc alloc;//funcion que se debe utilizar para crear nodos 
    TTFree release;//Guarda la función que libera los nodos.
    void *allocator_context;//Guarda información adicional para el sistema de memoria.
} TTree;

typedef enum TTResult { TT_OOM = -1, TT_DUPLICATE = 0, TT_INSERTED = 1 } TTResult;

typedef bool (*TTVisit)(int32_t, void *);
/* I1: estructura y construcción. NULL alloc/free selecciona malloc/free. */
void tt_init(TTree *, TTAlloc, TTFree, void *);
void tt_clear(TTree *);

TTResult tt_build(TTree *, const int32_t *, size_t);

size_t tt_memory_bytes(const TTree *);
/* I2: búsqueda, rango, recorrido y validación. */
bool tt_contains(const TTree *, int32_t);//Pregunta si existe una clave.

bool tt_visit_range(const TTree *, int32_t, int32_t, TTVisit, void *);

bool tt_validate(const TTree *, char *, size_t);
/* I3: inserción única. No cambia el árbol ante duplicados o malloc fallido. */
TTResult tt_insert(TTree *, int32_t);


#endif
