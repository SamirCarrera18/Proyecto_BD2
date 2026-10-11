# T-Tree académico en C para PostgreSQL 18.6

Implementación completa del alcance de indexación hasta el parcial: construcción,
búsqueda por igualdad, inserción, balanceo, rango inclusivo, validación y funciones
SQL que ejecutan el código C dentro del servidor PostgreSQL.

## 1. Alcance y decisiones

- Claves `INTEGER` de 32 bits, únicas. Se rechazan NULL y duplicados al construir;
  insertar un duplicado devuelve `false` sin cambiar el árbol.
- Nodos con hasta 16 claves ordenadas; mínimo interno 15. Hojas y nodos con un
  único hijo pueden tener entre 1 y 16. Balance de alturas AVL.
- Inserción en nodo acotador; al llenarse, desplaza su mínimo hacia el predecesor.
  Incluye redistribución especial antes de rotaciones dobles LR/RL cuando el nodo
  que asciende es hoja. No se sustituye el T-Tree por una biblioteca de árboles.
- La construcción usa inserciones sucesivas; no requiere claves previamente ordenadas.
- El índice guarda claves, no filas ni identificadores físicos de tuplas.
  Ofrece existencia y recuperación de claves por rango.
- La integración es una extensión de funciones SQL explícitas, **no** un método
  nativo `CREATE INDEX ... USING ttree`. El planificador no la selecciona automáticamente.
- Un árbol por transacción y conexión. Usar `BEGIN`, construir, consultar/insertar
  y finalizar con `COMMIT`. El árbol se libera al terminar la transacción.
- Después de reiniciar, reconectar, COMMIT o ROLLBACK, reconstruir desde la tabla.
  Si se revierte un SAVEPOINT, se invalida conservadoramente todo el árbol y hay
  que reconstruirlo. No devuelve silenciosamente un árbol con inserciones revertidas.
- `ttree_insert(k)` modifica **solo el índice en memoria**. Para registrar un dato
  en la tabla, hacer también `INSERT INTO tabla VALUES(k)` en la misma transacción,
  como muestra `sql/demo.sql`. UPDATE, DELETE, TRUNCATE o escrituras directas no
  actualizan el índice: reconstruirlo antes de seguir consultándolo.
- Construir toma un bloqueo SHARE de la tabla hasta finalizar la transacción para
  evitar escrituras concurrentes de otras sesiones. No es concurrencia de producción.
- No incluye eliminación, WAL propio, índice persistente, integración MVCC completa
  ni distribución de tres nodos. La rúbrica reserva distribución para la segunda etapa.

## 2. Cinco partes del código

| Persona | Archivo | Responsabilidad |
|---|---|---|
| I1 | `src/ttree_nodes.c` | Nodos, memoria, construcción y tamaño |
| I2 | `src/ttree_search.c` | Búsqueda, rangos y validación |
| I3 | `src/ttree_insert.c` | Inserción y manejo de nodos llenos |
| I4 | `src/ttree_balance.c` | Alturas, rotaciones y redistribución |
| I5 | `src/pg_ttree.c` | Funciones PostgreSQL, SPI y ciclo de vida |

Contrato compartido: `include/ttree.h` e `include/ttree_internal.h`.
I1 construye mediante I3; I3 llama a I4; I2 consulta y valida los nodos;
I5 lee las claves de la tabla con SPI y utiliza esos mismos módulos.
Pruebas, datos, benchmark y documentación corresponden al equipo completo.

## 3. Ejecutar con Docker (Windows, macOS o Linux)

Requisito: Docker con soporte de contenedores Linux y Docker Compose.
Abrir una terminal **dentro de esta carpeta**, donde están Dockerfile y compose.yaml.
Las contraseñas del ejemplo son exclusivamente para un entorno local académico.

```sh
docker compose up --build -d --wait
docker compose exec db psql -U proyecto -d proyecto -v ON_ERROR_STOP=1 -f /opt/ttree/sql/demo.sql
docker compose exec db psql -U proyecto -d proyecto -v ON_ERROR_STOP=1 -f /opt/ttree/tests/test_pg.sql
```

La imagen instala los compiladores y encabezados de PostgreSQL, compila los cinco
módulos e instala la extensión. `CREATE EXTENSION ttree` habilita sus funciones
por base de datos. No requiere C++ ni instalar PostgreSQL directamente en Windows.

Para ejecutar operaciones manuales en **una misma sesión**:

```sh
docker compose exec db psql -U proyecto -d proyecto
```

```sql
CREATE EXTENSION IF NOT EXISTS ttree;
CREATE TABLE mis_claves(id integer NOT NULL);
INSERT INTO mis_claves VALUES (10),(20),(30),(40);
BEGIN;
SELECT ttree_build('mis_claves','id');
SELECT ttree_contains(20); -- true
SELECT ttree_contains(25); -- false
INSERT INTO mis_claves VALUES (25);
SELECT ttree_insert(25);   -- true
SELECT * FROM ttree_range(15,35); -- 20,25,30
SELECT ttree_validate();
SELECT ttree_stats();
COMMIT;
```

En pgAdmin u otro cliente, conectar a `localhost:5433`, base/usuario `proyecto`,
contraseña `proyecto_local`. Mantener todas las operaciones dentro del mismo BEGIN.

```sh
docker compose stop
docker compose start
```

El volumen conserva las tablas; el árbol se reconstruye. Tras cambiar el código:
`docker compose up --build -d --wait`, abrir nueva conexión y volver a construir el árbol.
El archivo `compose.yaml` cumple la función de `docker-compose.yml` mencionada en
la rúbrica; Docker Compose reconoce ese nombre.

## 4. Pruebas del núcleo sin PostgreSQL

En Linux, con GCC o Clang y una terminal POSIX:

```sh
sh scripts/test_core.sh
```

Ejecuta AddressSanitizer y UndefinedBehaviorSanitizer para capacidades 2, 3, 4, 16
 y 64. Comprueba claves ascendentes, descendentes y aleatorias, extremos int32,
duplicados, rango, orden global, alturas, ocupación interna, contadores y fallos
simulados del asignador. La construcción fallida conserva el árbol previo.

También se puede ejecutar dentro del contenedor:

```sh
docker compose exec db sh scripts/test_core.sh
```

Si el sistema impide LeakSanitizer por restricciones de ptrace, se puede ejecutar
`ASAN_OPTIONS=detect_leaks=0 sh scripts/test_core.sh`; eso desactiva únicamente
la detección de fugas, no AddressSanitizer ni UndefinedBehaviorSanitizer.

## 5. Benchmark preliminar

El núcleo y extensión son C. Los scripts auxiliares usan Python, permitido por la rúbrica.
Para una primera ejecución manejable: 10 000 registros, 1 000 consultas exitosas,
1 000 ausentes, 10% de inserciones y 5 repeticiones:

```sh
docker compose exec db python3 scripts/benchmark.py --n 10000 --queries 1000 --repetitions 5
docker compose cp db:/opt/ttree/results ./results
```

Se generan `raw.csv` y `metadata.json`, con semilla, versión y configuración.
Cada repetición utiliza los mismos datos y cargas para: acceso sin índice, B-tree
nativo y T-Tree. Las tablas son temporales y no alteran las tablas del usuario.
Las consultas son de existencia; no se compara recuperación de filas contra una
mera búsqueda de claves. Se verifica el número de aciertos y se calienta cada carga.
El orden de las alternativas se mezcla de forma reproducible.

Para D2 y W1/W2/W3 a escala de la etapa experimental, repetir con `--n 100000`,
`--n 500000` y `--n 1000000`, usando `--queries 10000`. El caso sin índice puede
ser muy lento. No ejecutar esas escalas para una demo rápida.

Interpretación:
- Los tiempos son milisegundos del lote completo, con una llamada cliente por lote.
- Se reporta mediana de al menos 5 repeticiones para reducir el efecto de valores extremos.
- Construcción T-Tree incluye lectura SPI + inserciones; B-tree usa CREATE INDEX.
- Inserción T-Tree mide INSERT en tabla + actualización explícita del árbol; B-tree
  actualiza su índice automáticamente. No omitir esta diferencia en el informe.
- `index_bytes` de T-Tree contabiliza estructuras C, sin overhead del asignador/SPI;
  B-tree usa pg_relation_size, almacenamiento por páginas. No son medidas idénticas
  de RAM ni de disco. Informar esta limitación al compararlas.
- Las búsquedas SQL y las funciones C tienen distintos costos de llamada. Esta prueba
  mide las rutas integradas, no aísla únicamente la complejidad de los algoritmos.
- Este benchmark es preliminar: falta automatizar rangos W4 y carga sesgada D3,
  comparar cargas favorables/neutras y desarrollar el escenario distribuido final.

Generador independiente para D1 o D2:

```sh
python3 scripts/generate_data.py --mode random --n 100000 --seed 20260911 > datos.csv
```

## 6. Complejidad y defensa

Sea N la cantidad de claves y C la capacidad fija del nodo:
- Búsqueda: O(log N + log C).
- Inserción: O(log N + C), incluido desplazamiento dentro de un nodo; con C fijo, O(log N).
- Construcción por inserciones: O(N log N) con C fijo.
- Validación y recorrido completo: O(N); rango: O(log N + K) con C fijo y K resultados.
- Memoria del núcleo: proporcional al número de nodos; SPI añade memoria temporal
  durante construcción. Este diseño no promete superar al B-tree de PostgreSQL.

Todos deben poder explicar los cinco módulos y las limitaciones. Este código fue
preparado con asistencia de IA: declararlo según la rúbrica y estudiar sus pruebas.
No se incluye una propuesta inicial de dos páginas ni un informe con resultados
experimentales inventados. Revisar `docs/VALIDACION.md` para lo comprobado en la entrega.

## 7. Fuentes

- Lehman y Carey, *A Study of Index Structures for Main Memory Database Management
  Systems*, VLDB 1986, sección 3.2 (fundamento del T-Tree):
  https://www.vldb.org/conf/1986/P294.PDF
- Funciones C PostgreSQL 18:
  https://www.postgresql.org/docs/18/xfunc-c.html
- Interfaz SPI: https://www.postgresql.org/docs/18/spi.html
- Compilación PGXS: https://www.postgresql.org/docs/18/extend-pgxs.html
- Docker: https://docs.docker.com/get-started/docker-overview/

Se implementan los algoritmos en este proyecto; no se incorporó código de una
biblioteca externa de árboles. Las referencias explican el diseño, no sustituyen
la revisión y comprensión del equipo.
