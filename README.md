# Proyecto BD2 — T-Tree en C para PostgreSQL 18.6


El proyecto cubre la **Etapa I**: construcción de la estructura, búsqueda por igualdad, inserción, balanceo, búsqueda por rango, validación, integración con PostgreSQL, pruebas y comparación preliminar con B-tree.

## Integrantes y responsabilidades

| Integrante | Responsabilidad principal |
|---|---|
| Iván | Estructuras, memoria y construcción |
| Sebastian | Búsqueda, rangos y validación |
| Kevin | Inserción, desbordamiento y benchmark |
| Samir | Balance, pruebas y entorno reproducible |
| Marcelo | Integración con PostgreSQL |

Aunque cada integrante tiene un módulo principal, todos deben conocer el flujo general del sistema.

## Estructura del proyecto

```text
Proyecto_BD2/
├── include/
│   ├── ttree.h
│   └── ttree_internal.h
├── src/
│   ├── pg_ttree.c
│   ├── ttree_balance.c
│   ├── ttree_insert.c
│   ├── ttree_nodes.c
│   └── ttree_search.c
├── sql/
│   ├── demo.sql
│   └── ttree--1.0.sql
├── tests/
│   ├── test_core.c
│   └── test_pg.sql
├── scripts/
│   ├── benchmark.py
│   ├── generate_data.py
│   └── test_core.sh
├── Dockerfile
├── compose.yaml
├── Makefile
├── ttree.control
└── README.md
```

## Funcionalidades principales

- Construcción del T-Tree desde una columna `INTEGER` de PostgreSQL.
- Búsqueda por igualdad.
- Inserción de claves.
- Búsqueda por rango.
- Balanceo mediante rotaciones.
- Validación de orden, ocupación, alturas, balance y contadores.
- Estadísticas del árbol.
- Integración con PostgreSQL mediante funciones C y SPI.

## Compilación y ejecución

Requisito: Docker con soporte para contenedores Linux.

```bash
docker compose up --build -d --wait
```

Verificar el servicio:

```bash
docker compose ps
```

## Demostración

```bash
docker compose exec -T db psql -U proyecto -d proyecto -v ON_ERROR_STOP=1 -f /opt/ttree/sql/demo.sql
```

## Pruebas de integración

```bash
docker compose exec -T db psql -U proyecto -d proyecto -v ON_ERROR_STOP=1 -f /opt/ttree/tests/test_pg.sql
```

Las pruebas cubren construcción, búsqueda, inserción, rangos, duplicados, `NULL`, `SAVEPOINT`, `ROLLBACK`, `COMMIT` y reconstrucción.

## Pruebas del núcleo C

```bash
docker compose exec db sh scripts/test_core.sh
```

Se prueban distintas capacidades de nodo y casos ascendentes, descendentes, aleatorios, duplicados, rangos, extremos de `int32_t` y fallos simulados de memoria.

En entornos donde LeakSanitizer genere problemas:

```bash
ASAN_OPTIONS=detect_leaks=0 sh scripts/test_core.sh
```

## Uso manual

```bash
docker compose exec db psql -U proyecto -d proyecto
```

Ejemplo:

```sql
CREATE EXTENSION IF NOT EXISTS ttree;

CREATE TABLE mis_claves(id integer NOT NULL);
INSERT INTO mis_claves VALUES (10), (20), (30), (40);

BEGIN;

SELECT ttree_build('mis_claves', 'id');
SELECT ttree_contains(20);
SELECT ttree_contains(25);

INSERT INTO mis_claves VALUES (25);
SELECT ttree_insert(25);

SELECT * FROM ttree_range(15, 35);
SELECT ttree_validate();
SELECT ttree_stats();

COMMIT;
```

## Flujo general

```text
Docker
  ↓
PostgreSQL
  ↓
CREATE EXTENSION ttree
  ↓
ttree_build()
  ↓
pg_ttree.c + SPI
  ↓
núcleo T-Tree en C
  ↓
tt_insert() / tt_rebalance()
  ↓
ttree_contains() / ttree_range()
```

## Benchmark preliminar

```bash
docker compose exec db python3 scripts/benchmark.py --n 10000 --queries 1000 --repetitions 5
```

Compara:

- acceso sin índice;
- B-tree de PostgreSQL;
- T-Tree implementado.

## Limitaciones

Esta implementación es académica y **no es un método de acceso nativo** de PostgreSQL.

- No utiliza `CREATE INDEX ... USING ttree`.
- El planificador no selecciona automáticamente el T-Tree.
- El árbol permanece en memoria durante la transacción.
- `ttree_insert()` modifica el T-Tree, no la tabla PostgreSQL.
- No se implementan WAL propio, persistencia, MVCC completo, VACUUM especializado ni eliminación.

## Fuentes

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
