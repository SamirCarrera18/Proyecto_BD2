\set ON_ERROR_STOP on
CREATE EXTENSION IF NOT EXISTS ttree;
BEGIN;
CREATE TEMP TABLE test_keys(id integer NOT NULL);
INSERT INTO test_keys SELECT g*2 FROM generate_series(-20000,20000)g ORDER BY md5(g::text);
SELECT ttree_build('test_keys','id');
DO $$
DECLARE k integer; result integer[];
BEGIN
 FOR k IN -40000..40000 LOOP
  IF ttree_contains(k) != (k%2=0) THEN RAISE EXCEPTION 'BÃºsqueda incorrecta %',k; END IF;
 END LOOP;
 IF NOT ttree_validate() THEN RAISE EXCEPTION 'Ãrbol invÃ¡lido'; END IF;
 SELECT array_agg(v ORDER BY v) INTO result FROM ttree_range(-4,4)v;
 IF result != ARRAY[-4,-2,0,2,4] THEN RAISE EXCEPTION 'Rango incorrecto'; END IF;
 IF NOT ttree_insert(3) OR ttree_insert(3) THEN RAISE EXCEPTION 'InserciÃ³n incorrecta'; END IF;
 IF NOT ttree_contains(3) THEN RAISE EXCEPTION 'Clave insertada ausente'; END IF;
 PERFORM ttree_validate();
END $$;
SAVEPOINT before_insert;
SELECT ttree_insert(5);
ROLLBACK TO SAVEPOINT before_insert;
-- Un aborto de subtransacción invalida todo el árbol, no deja datos revertidos.
DO $$ BEGIN
 BEGIN PERFORM ttree_contains(5); RAISE EXCEPTION 'Se esperaba invalidación';
 EXCEPTION WHEN OTHERS THEN
  IF SQLSTATE <> 'XX000' THEN RAISE; END IF;
 END;
END $$;
SELECT ttree_build('test_keys','id');
DO $$ BEGIN IF ttree_contains(3) OR ttree_contains(5) THEN RAISE EXCEPTION 'Estado obsoleto'; END IF; END $$;
COMMIT;
DO $$ BEGIN
 BEGIN PERFORM ttree_contains(2); RAISE EXCEPTION 'Estado persistiÃ³ tras COMMIT';
 EXCEPTION WHEN OTHERS THEN
  IF SQLSTATE <> 'XX000' THEN RAISE; END IF;
 END;
END $$;
BEGIN;
CREATE TEMP TABLE bad_keys(id integer);
INSERT INTO bad_keys VALUES(1),(1);
DO $$ BEGIN
 BEGIN PERFORM ttree_build('bad_keys','id'); RAISE EXCEPTION 'Duplicados aceptados';
 EXCEPTION WHEN OTHERS THEN
  IF SQLERRM NOT LIKE 'Clave duplicada:%' THEN RAISE; END IF;
 END;
END $$;
TRUNCATE bad_keys; INSERT INTO bad_keys VALUES(NULL);
DO $$ BEGIN
 BEGIN PERFORM ttree_build('bad_keys','id'); RAISE EXCEPTION 'NULL aceptado';
 EXCEPTION WHEN OTHERS THEN
  IF SQLERRM != 'No se admiten claves NULL' THEN RAISE; END IF;
 END;
END $$;
TRUNCATE bad_keys;
SELECT ttree_build('bad_keys','id');
DO $$ BEGIN IF ttree_contains(1) OR NOT ttree_validate() THEN RAISE EXCEPTION 'VacÃ­o incorrecto'; END IF; END $$;
SELECT ttree_reset();
COMMIT;
SELECT 'OK: integración, búsqueda, inserciÃ³n, rangos, duplicados, NULL, rollback y commit' AS resultado;


