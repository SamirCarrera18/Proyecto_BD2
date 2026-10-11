# Validación de esta entrega

## Ejecutado

- Núcleo compilado en C11 con GCC, `-Wall -Wextra -Werror -pedantic`.
- AddressSanitizer y UndefinedBehaviorSanitizer: sin errores detectados.
- Capacidades 2, 3, 4, 16 y 64.
- En cada capacidad: 30 000 claves ascendentes, 30 000 descendentes y 30 000
  aleatorias; comprobación posterior de todas las búsquedas presentes/ausentes
  y duplicados. Validación periódica y final de orden, alturas, balance y ocupación.
- Recorrido ordenado, rangos, INT32_MIN e INT32_MAX.
- Fallos simulados de asignación durante inserción y construcción; árbol original
  intacto y contador de asignaciones liberadas comprobado en esa prueba.
- Compilación PGXS de la extensión contra PostgreSQL 18.6 compilado desde sus fuentes
  oficiales. Última compilación del adaptador con `-Werror`, sin advertencias.
- Símbolos exportados de las funciones SQL y `_PG_init` comprobados con `nm`.
- Sintaxis Python de los generadores y benchmark, y sintaxis shell verificadas.

LeakSanitizer se desactivó porque el entorno restringe su inspección de procesos.
No se afirma haber realizado una comprobación general de fugas mediante esa herramienta.

## Incluido, pendiente de ejecutar en el entorno del grupo

- `tests/test_pg.sql`: pruebas funcionales dentro de PostgreSQL.
- `sql/demo.sql`: demostración SQL.
- `scripts/benchmark.py`: comparación preliminar con cinco repeticiones.
- Construcción y arranque del Dockerfile/Compose.

En este entorno no estaba disponible Docker. PostgreSQL se compiló, pero no pudo
arrancarse bajo una cuenta no privilegiada debido a las restricciones de cambio
de usuario. Por tanto, compilar la extensión NO equivale a haber validado su
comportamiento dentro de un servidor en ejecución. No hay mediciones PostgreSQL
ni resultados de benchmark fabricados en esta entrega.
