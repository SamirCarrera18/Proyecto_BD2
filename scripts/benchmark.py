#!/usr/bin/env python3
"""Comparación preliminar reproducible; tablas temporales, sin tocar tablas del usuario."""
import argparse, csv, io, json, os, random, statistics, time
from pathlib import Path
import psycopg2

def timed(cursor, query):
    start = time.perf_counter()
    cursor.execute(query)
    elapsed = (time.perf_counter() - start) * 1000
    return elapsed

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dsn', default=os.getenv('DATABASE_URL', 'dbname=proyecto user=proyecto'))
    ap.add_argument('--n', type=int, default=10000)
    ap.add_argument('--queries', type=int, default=1000, help='Cantidad por carga: exitosas y ausentes')
    ap.add_argument('--repetitions', type=int, default=5)
    ap.add_argument('--seed', type=int, default=20260911)
    ap.add_argument('--out', default='results')
    args = ap.parse_args()
    if not 1 <= args.n <= 1000000 or args.queries < 1 or args.repetitions < 5:
        ap.error('n: 1..1000000; queries >= 1; repetitions >= 5')
    rng = random.Random(args.seed)
    # D2: únicos, aleatorios y reproducibles; pares => impares siempre ausentes.
    keys = [2*x for x in rng.sample(range(1, 10000001), args.n)]
    hits = [rng.choice(keys) for _ in range(args.queries)]
    misses = [rng.choice(keys)+1 for _ in range(args.queries)]
    extra = [20000002+2*i for i in range(max(1,args.n//10))]
    out = Path(args.out); out.mkdir(parents=True,exist_ok=True)
    rows=[]; metadata={}
    with psycopg2.connect(args.dsn) as conn:
        with conn.cursor() as cur:
            cur.execute('CREATE EXTENSION IF NOT EXISTS ttree')
        conn.commit()
        for repetition in range(args.repetitions):
            with conn.cursor() as cur:
                cur.execute('SELECT version()');metadata['version']=cur.fetchone()[0]
                for setting in ['shared_buffers','work_mem','max_parallel_workers_per_gather','jit']:
                    cur.execute('SHOW '+setting);metadata[setting]=cur.fetchone()[0]
                for name in ['bench_seq','bench_bt','bench_tt']:
                    cur.execute(f'CREATE TEMP TABLE {name}(id integer NOT NULL) ON COMMIT DROP')
                    cur.copy_from(io.StringIO(''.join(f'{k}\n' for k in keys)),name,columns=('id',))
                    cur.execute(f'ANALYZE {name}')
                for name, workload in [('work_hits',hits),('work_misses',misses),('work_insert',extra)]:
                    cur.execute(f'CREATE TEMP TABLE {name}(id integer) ON COMMIT DROP')
                    cur.copy_from(io.StringIO(''.join(f'{k}\n' for k in workload)),name,columns=('id',))
                build_bt=timed(cur,'CREATE INDEX bench_bt_idx ON bench_bt USING btree(id)')
                build_tt=timed(cur,"SELECT ttree_build('bench_tt','id')")
                assert cur.fetchone()[0]==args.n
                cur.execute("SELECT pg_relation_size('bench_bt_idx')");bytes_bt=cur.fetchone()[0]
                cur.execute('SELECT ttree_stats()');stats=cur.fetchone()[0]
                bytes_tt=int(stats.split('bytes_core=')[1])
                # Mismo bucle servidor y orden de claves en todos los modos.
                cur.execute('''CREATE OR REPLACE FUNCTION pg_temp.bench_lookup(mode text, successful boolean)
                RETURNS bigint LANGUAGE plpgsql AS $$
                DECLARE k integer; found boolean; total bigint:=0;
                BEGIN
                  FOR k IN SELECT id FROM work_hits WHERE successful
                           UNION ALL SELECT id FROM work_misses WHERE NOT successful LOOP
                    IF mode='seq' THEN SELECT EXISTS(SELECT 1 FROM bench_seq WHERE id=k) INTO found;
                    ELSIF mode='bt' THEN SELECT EXISTS(SELECT 1 FROM bench_bt WHERE id=k) INTO found;
                    ELSE found:=ttree_contains(k); END IF;
                    IF found THEN total:=total+1; END IF;
                  END LOOP;
                  RETURN total;
                END $$''')
                cur.execute('''CREATE OR REPLACE FUNCTION pg_temp.bench_insert(mode text)
                RETURNS void LANGUAGE plpgsql AS $$ DECLARE k integer; BEGIN
                  FOR k IN SELECT id FROM work_insert LOOP
                    IF mode='seq' THEN INSERT INTO bench_seq VALUES(k);
                    ELSIF mode='bt' THEN INSERT INTO bench_bt VALUES(k);
                    ELSE INSERT INTO bench_tt VALUES(k); PERFORM ttree_insert(k); END IF;
                  END LOOP;
                END $$''')
                order=['seq','bt','tt'];random.Random(args.seed+repetition).shuffle(order)
                for mode in order:
                    row={'repetition':repetition+1,'mode':mode,'n':args.n,'queries_per_workload':args.queries,
                         'insert_count':len(extra),'build_ms':{'seq':0,'bt':build_bt,'tt':build_tt}[mode],
                         'index_bytes':{'seq':0,'bt':bytes_bt,'tt':bytes_tt}[mode]}
                    for successful,label in [(True,'hit_ms'),(False,'miss_ms')]:
                        query=f"SELECT pg_temp.bench_lookup('{mode}',{str(successful).lower()})"
                        cur.execute(query)  # calentamiento explícito de esta carga
                        assert cur.fetchone()[0]==(args.queries if successful else 0)
                        row[label]=timed(cur,query)
                        assert cur.fetchone()[0]==(args.queries if successful else 0)
                    row['insert_ms']=timed(cur,f"SELECT pg_temp.bench_insert('{mode}')")
                    rows.append(row)
                cur.execute('SELECT ttree_validate()');assert cur.fetchone()[0]
                for name in ['bench_seq','bench_bt','bench_tt']:
                    cur.execute(f'SELECT count(*) FROM {name}');assert cur.fetchone()[0]==args.n+len(extra)
            conn.commit()  # libera árbol y elimina tablas temporales antes de repetir
    with (out/'raw.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=rows[0].keys());writer.writeheader();writer.writerows(rows)
    metadata.update(n=args.n,queries=args.queries,repetitions=args.repetitions,seed=args.seed,
                    workload='D2 uniforme, W1, W2, W3 (10%)',central_measure='mediana',
                    timing='ms del lote completo; reloj cliente, una llamada por lote',
                    memory_note='B-tree: pg_relation_size; T-Tree: bytes de estructuras C, sin overhead del allocator',
                    scope='preliminar; no ejecuta D1/D3 ni W4 ni escenario distribuido')
    (out/'metadata.json').write_text(json.dumps(metadata,indent=2,ensure_ascii=False))
    for mode in ['seq','bt','tt']:
        print(mode, {k:round(statistics.median(r[k] for r in rows if r['mode']==mode),3)
                     for k in ['build_ms','hit_ms','miss_ms','insert_ms','index_bytes']})
    print('Resultados:',out.resolve())
if __name__=='__main__':main()
