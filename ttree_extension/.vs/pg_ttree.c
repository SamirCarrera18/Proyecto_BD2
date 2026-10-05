/* Integrante 5. Extensión PostgreSQL: estado local a UNA transacción/backend.
 * Las operaciones del núcleo son C puro; PostgreSQL administra su memoria.
 */
#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "access/xact.h"
#include "catalog/pg_type_d.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "storage/lmgr.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "ttree.h"
#include <limits.h>
PG_MODULE_MAGIC;
PGDLLEXPORT void _PG_init(void);
PG_FUNCTION_INFO_V1(pg_ttree_build);
PG_FUNCTION_INFO_V1(pg_ttree_contains);
PG_FUNCTION_INFO_V1(pg_ttree_insert);
PG_FUNCTION_INFO_V1(pg_ttree_range);
PG_FUNCTION_INFO_V1(pg_ttree_validate);
PG_FUNCTION_INFO_V1(pg_ttree_stats);
PG_FUNCTION_INFO_V1(pg_ttree_reset);
static TTree active;
static MemoryContext tree_context=NULL;
static bool ready=false;
static void forget_tree(void) {
    ready=false; tree_context=NULL; memset(&active,0,sizeof(active));
}
static void on_transaction(XactEvent event,void *arg) {
    (void)arg;
    if (event==XACT_EVENT_COMMIT || event==XACT_EVENT_ABORT || event==XACT_EVENT_PREPARE)
        forget_tree(); /* TopTransactionContext libera todas las asignaciones. */
}
static void on_subtransaction(SubXactEvent event,SubTransactionId id,SubTransactionId parent,void *arg) {
    (void)id; (void)parent; (void)arg;
    if (event==SUBXACT_EVENT_ABORT_SUB) {
        /* Invalidación conservadora: jamás devolver claves de un SAVEPOINT revertido. */
        if (tree_context) MemoryContextDelete(tree_context);
        forget_tree();
    }
}
void _PG_init(void) {
    RegisterXactCallback(on_transaction,NULL);
    RegisterSubXactCallback(on_subtransaction,NULL);
}
static void *allocate_node(size_t bytes,void *ctx) { return MemoryContextAlloc((MemoryContext)ctx,bytes); }
static void free_node(void *p,void *ctx) { (void)ctx; pfree(p); }
static void require_tree(void) {
    if (!ready) ereport(ERROR,(errmsg("T-Tree no construido en esta transacción"),
        errhint("Ejecute BEGIN; SELECT ttree_build('tabla','columna'); antes de consultar o insertar.")));
}
Datum pg_ttree_build(PG_FUNCTION_ARGS) {
    Oid rel=PG_GETARG_OID(0);
    char *column=text_to_cstring(PG_GETARG_TEXT_PP(1));
    char *name, *schema, *query;
    AttrNumber attribute;
    uint64 i;
    /* Mantener estable la tabla frente a escrituras de otras sesiones hasta COMMIT. */
    LockRelationOid(rel,ShareLock);
    name=get_rel_name(rel);
    if (!name) ereport(ERROR,(errmsg("La relación no existe")));
    schema=get_namespace_name(get_rel_namespace(rel));
    attribute=get_attnum(rel,column);
    if (attribute<=0 || get_atttype(rel,attribute)!=INT4OID)
        ereport(ERROR,(errmsg("La columna debe existir y ser INTEGER (int4)")));
    if (tree_context) MemoryContextDelete(tree_context);
    forget_tree();
    tree_context=AllocSetContextCreate(TopTransactionContext,"Academic T-Tree",ALLOCSET_DEFAULT_SIZES);
    tt_init(&active,allocate_node,free_node,tree_context);
    query=psprintf("SELECT %s FROM %s",quote_identifier(column),quote_qualified_identifier(schema,name));
    if (SPI_connect()!=SPI_OK_CONNECT) elog(ERROR,"SPI_connect falló");
    if (SPI_execute(query,true,0)!=SPI_OK_SELECT) elog(ERROR,"Lectura de tabla falló");
    for(i=0;i<SPI_processed;++i) {
        bool is_null;
        Datum value=SPI_getbinval(SPI_tuptable->vals[i],SPI_tuptable->tupdesc,1,&is_null);
        TTResult result;
        if (is_null) ereport(ERROR,(errmsg("No se admiten claves NULL")));
        result=tt_insert(&active,DatumGetInt32(value));
        if(result==TT_DUPLICATE) ereport(ERROR,(errmsg("Clave duplicada: %d",DatumGetInt32(value))));
        if(result==TT_OOM) ereport(ERROR,(errmsg("Memoria insuficiente")));
        CHECK_FOR_INTERRUPTS();
    }
    SPI_finish();
    ready=true;
    PG_RETURN_INT64((int64)active.size);
}
Datum pg_ttree_contains(PG_FUNCTION_ARGS) {
    require_tree(); PG_RETURN_BOOL(tt_contains(&active,PG_GETARG_INT32(0)));
}
Datum pg_ttree_insert(PG_FUNCTION_ARGS) {
    TTResult result;
    require_tree();
    /* Inserta en la estructura, NO en la tabla SQL de origen. Véase demo.sql. */
    result=tt_insert(&active,PG_GETARG_INT32(0));
    if(result==TT_OOM) ereport(ERROR,(errmsg("Memoria insuficiente")));
    PG_RETURN_BOOL(result==TT_INSERTED);
}
static bool emit_key(int32_t key,void *context) {
    ReturnSetInfo *rsinfo=(ReturnSetInfo *)context;
    Datum values[1]; bool nulls[1]={false};
    values[0]=Int32GetDatum(key);
    tuplestore_putvalues(rsinfo->setResult,rsinfo->setDesc,values,nulls);
    CHECK_FOR_INTERRUPTS(); return true;
}
Datum pg_ttree_range(PG_FUNCTION_ARGS) {
    require_tree();
    InitMaterializedSRF(fcinfo,MAT_SRF_USE_EXPECTED_DESC);
    tt_visit_range(&active,PG_GETARG_INT32(0),PG_GETARG_INT32(1),emit_key,(ReturnSetInfo *)fcinfo->resultinfo);
    return (Datum)0;
}
Datum pg_ttree_validate(PG_FUNCTION_ARGS) {
    char message[160]; require_tree();
    if (!tt_validate(&active,message,sizeof(message))) ereport(ERROR,(errmsg("T-Tree inválido: %s",message)));
    PG_RETURN_BOOL(true);
}
Datum pg_ttree_stats(PG_FUNCTION_ARGS) {
    require_tree();
    PG_RETURN_TEXT_P(cstring_to_text(psprintf("keys=%zu nodes=%zu height=%d capacity=%d min_internal=%d bytes_core=%zu",
       active.size,active.nodes,active.root?active.root->height:0,TT_CAPACITY,TT_MIN_INTERNAL,tt_memory_bytes(&active))));
}
Datum pg_ttree_reset(PG_FUNCTION_ARGS) {
    if(tree_context) MemoryContextDelete(tree_context);
    forget_tree(); PG_RETURN_VOID();
}
