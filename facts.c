

/*  
 * $$LICENCE$$
 *
 */

/* FIXME: We'd like to see an enum here but that would
 * make the interface to SQL harder.
 */
#define  FACT_ERROR    0
#define  FACT_CREATED  1
#define  FACT_UPDATED  2
#define  FACT_DELETED  3

/* This is void* but we want to distinguish it here */
typedef void *pg_plan;


#define QUERY_FIND \
          "SELECT * FROM facts WHERE uri = $1 " \
          " AND namespace = $2 AND name = $3 "

#define QUERY_INSERT \
          "INSERT INTO facts VALUES( $1, $2, $3, $4 )"

#define QUERY_UPDATE \
          "UPDATE facts SET value = $4 WHERE " \
          " uri = $1 AND namespace = $2 AND name = $3 "

#include "executor/spi.h"



PG_FUNCTION_INFO_V1(assert);
Datum
assert(PG_FUNCTION_ARGS)
{
  Datum           values[4];
  static pg_plan  find_plan, insert_plan, update_plan ;
  static Oid      ctypes[4] = {TEXTOID, TEXTOID, TEXTOID, TEXTOID};
  int             res = -1;
  int             status;

  /* Check for not-NULL */
  if(PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||PG_ARGISNULL(3)) {
    elog(ERROR, "function assert(): Can not assert NULL knowlege");
  }

  values[0] = PG_GETARG_DATUM(0);
  values[1] = PG_GETARG_DATUM(1);
  values[2] = PG_GETARG_DATUM(2);
  values[3] = PG_GETARG_DATUM(3);

  status = 0;
  if(SPI_connect() != SPI_OK_CONNECT)
    {
      elog(ERROR, "function facts: SPI_connect failed");
      return FACT_ERROR;
    }

  if(!find_plan)
    {
      pg_plan pl;

      pl = SPI_prepare(QUERY_FIND, 4, ctypes);
      if (pl == NULL) {
	elog(ERROR, "function facts: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
      elog(NOTICE, "Saved query_find plan for function assert()");
      find_plan = SPI_saveplan(pl);
    }
  if(!insert_plan)
    {
      pg_plan pl;

      pl = SPI_prepare(QUERY_INSERT, 4, ctypes);
      if (pl == NULL) {
	elog(ERROR, "function facts: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
      elog(NOTICE, "Saved query_insert plan for function assert()");
      insert_plan = SPI_saveplan(pl);
    }
  if(!update_plan)
    {
      pg_plan pl;

      pl = SPI_prepare(QUERY_UPDATE, 4, ctypes);
      if (pl == NULL) {
	elog(ERROR, "function facts: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
      elog(NOTICE, "Saved query_update plan for function assert()");
      update_plan = SPI_saveplan(pl);
    }

  /* We have a connection to the server core with our own mamory context
   * and a precompiled plan - let's see whether we allready have a similar
   * fact in the databse.
   */

  res = SPI_execp(find_plan, values, "____", 0);
  if (res != SPI_OK_SELECT) {
    elog(ERROR, "function facts: could not check fact. SPI_exep returned %d", res);
    goto FINISH;
  }

  elog(NOTICE, "QUERY ARGS: %d", fcinfo->nargs);


  /* Check the number of tuples found */
  switch(SPI_processed) {
  case 0:   /* Nothing similar found -> insert new fact */
    res = SPI_execp(insert_plan, values, "____", 0);
    elog(NOTICE, "Inserting fact ...");
    goto FINISH;
    break;
  case 1:   /* Similar fact found -> update found fact */
    res = SPI_execp(update_plan, values, "____", 0);
    elog(NOTICE, "Updating fact ...");
    goto FINISH;
    break;
  default: /* More than one fact? Gosh! */
    elog(ERROR, "function facts: multiple facts found (%d). Please repair.", SPI_processed);
    goto FINISH;
  }
  
 FINISH:
  SPI_finish();
  PG_RETURN_INT32(status);
}
