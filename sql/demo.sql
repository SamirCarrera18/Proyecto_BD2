\set ON_ERROR_STOP on
CREATE EXTENSION IF NOT EXISTS ttree;
BEGIN;
CREATE TEMP TABLE demo_keys (id integer NOT NULL);
INSERT INTO demo_keys SELECT g*2 FROM generate_series(1,10000) AS g;
SELECT ttree_build('demo_keys','id');
SELECT ttree_contains(100) AS existente, ttree_contains(101) AS ausente;
-- Dos acciones explícitas: tabla persistente/temporal + estructura en memoria.
INSERT INTO demo_keys VALUES (101);
SELECT ttree_insert(101) AS insertada, ttree_insert(101) AS duplicada;
SELECT ttree_contains(101) AS encontrada_despues;
SELECT * FROM ttree_range(95,105);
SELECT ttree_validate(),ttree_stats();
DO $$ BEGIN
 IF NOT ttree_contains(101) OR ttree_contains(999999) THEN RAISE EXCEPTION 'Demo incorrecta'; END IF;
END $$;
COMMIT;
-- Al terminar la transacción, el árbol se libera. La tabla aún existe en esta sesión.
BEGIN;
SELECT ttree_build('demo_keys','id');
SELECT ttree_contains(101) AS reconstruccion_correcta;
COMMIT;
