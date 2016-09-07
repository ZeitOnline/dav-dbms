/* Filespec: $Id$ */

/* FIXME:
 * - declare facts_uri datatype as TOASTabel (
 * - use pg_detoast_datum() on all arguments that are facts_uri
 */

/*
 * $$LICENCE$$
 *
 */

#include "postgres.h"
#include "executor/spi.h"
#include "facts.h"
#include <string.h>

/* NOTE: This is new for Postgres >= 8.2 */
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
 #include <fmgr.h>
*/


/* This is void* but we want to distinguish it here */
typedef void *pg_plan;

/* Read a facts_uri
 */
PG_FUNCTION_INFO_V1(facts_uri_in);
Datum
facts_uri_in(PG_FUNCTION_ARGS)
{
  char	   *inputText = PG_GETARG_CSTRING(0);
  text	   *result;
  int	    len;


  len = strlen(inputText);
  result = (text *) palloc(len + VARHDRSZ);
  // NOT WORKING IN 8.3:
  //VARATT_SIZEP(result) =   len + VARHDRSZ;
  SET_VARSIZE(result, len);
  inputText[len] = '\0';
  memcpy(VARDATA(result), inputText, len);

  PG_RETURN_TEXT_P(result);
}

/* Write a facts_uri */
PG_FUNCTION_INFO_V1(facts_uri_out);
Datum
facts_uri_out(PG_FUNCTION_ARGS)
{
  text	   *t = PG_GETARG_TEXT_P(0);
  int	    len;
  char	   *result;

	len    = VARSIZE(t) - VARHDRSZ;
	result = (char *) palloc(len + 1);
	memcpy(result, VARDATA(t), len);
	result[len] = '\0';

	PG_RETURN_CSTRING(result);
}

/* Compare two URIs
 * For now we just compare the string representation.
 * NOTE: do we need to check for NULL values?
 */
PG_FUNCTION_INFO_V1(facts_uri_eq);
Datum
facts_uri_eq(PG_FUNCTION_ARGS)
{
  size_t   lenl, lenr;
  text    *l, *r;
  char    *left, *right;
  int      res  = 0;

  l = PG_GETARG_TEXT_P(0);
  r = PG_GETARG_TEXT_P(1);

  lenl  = VARSIZE(l) - VARHDRSZ;
  lenr  = VARSIZE(r) - VARHDRSZ;
  left  = VARDATA(l);
  right = VARDATA(r);

  if (lenl != lenr)
    PG_RETURN_BOOL(0);

  res = strncmp(left, right, lenl);
  if(res)
    PG_RETURN_BOOL(0);
  PG_RETURN_BOOL(1);
}

PG_FUNCTION_INFO_V1(facts_uri_neq);
Datum
facts_uri_neq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(1);
}

PG_FUNCTION_INFO_V1(facts_uri_lt);
Datum
facts_uri_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(1);
}

PG_FUNCTION_INFO_V1(facts_uri_le);
Datum
facts_uri_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(1);
}

PG_FUNCTION_INFO_V1(facts_uri_gt);
Datum
facts_uri_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(1);
}

PG_FUNCTION_INFO_V1(facts_uri_ge);
Datum
facts_uri_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(1);
}

/* Convert facts_uri -> text */
PG_FUNCTION_INFO_V1(facts_uri2text);
Datum
facts_uri2text(PG_FUNCTION_ARGS)
{
  Datum	    uri = PG_GETARG_DATUM(0);
  text	   *result;
  char	   *str;
  int	    len;

  str = DatumGetCString(DirectFunctionCall1(facts_uri_out, uri));

  len = (strlen(str) + VARHDRSZ);

  result = palloc(len);

  //Not working in 8.3:
  //VARATT_SIZEP(result) = len;
  SET_VARSIZE(result, len);
  memmove(VARDATA(result), str, (len - VARHDRSZ));
  pfree(str);

  PG_RETURN_TEXT_P(result);
}

/* Convert text -> facts_uri */
/* WARNING: this is _BROKEN_ !!! */
PG_FUNCTION_INFO_V1(facts_text2uri);
Datum
facts_text2uri(PG_FUNCTION_ARGS)
{
  text   *textin = PG_GETARG_TEXT_P(0);
  text   *result;
  int	  len;

  len = VARSIZE(textin) - VARHDRSZ;
  result = (text *) palloc(len + VARHDRSZ);
  //Not working in 8.3:
  //VARATT_SIZEP(result) =   len + VARHDRSZ;
  SET_VARSIZE(result, len);
  memcpy(VARDATA(result), textin, len);
  PG_RETURN_TEXT_P(result);
}

/*#########################################################[ Prolog-like Functions ]##*/

#define BATCHSIZE 10

#define QUERY_FIND \
          "SELECT * FROM facts WHERE uri = $1 " \
          " AND namespace = $2 AND name = $3 "

#define QUERY_INSERT \
          "INSERT INTO facts VALUES( $1, $2, $3, $4 )"

#define QUERY_UPDATE \
          "UPDATE facts SET value = $4 WHERE " \
          " uri = $1 AND namespace = $2 AND name = $3 "

#define QUERY_FORGET \
          "DELETE FROM facts WHERE uri = $1 AND namespace = $2 AND name = $3"

/*######################################################[ Resouce Deletion Support ]##*/

#define QUERY_DELETE_RESOURCE \
          "DELETE FROM facts WHERE uri = $1"

#define QUERY_DELETE_COLLECTION \
          "DELETE FROM facts WHERE uri like $1||'%'"

/*#####################################################[ Resouce Copy/Move Support ]##*/


#define QUERY_COPY_RESOURCE \
          "HONK! DELETE FROM facts WHERE uri = $1"

#define QUERY_COPY_COLLECTION \
          "HONK! DELETE FROM facts WHERE uri like $1||'%'"

#define QUERY_MOVE_RESOURCE \
          "HONK! DELETE FROM facts WHERE uri = $1"

#define QUERY_MOVE_COLLECTION \
          "HONK! DELETE FROM facts WHERE uri like $1||'%'"


/* NOTE: we use an explicit field name in the following query to save us
 * the touble of calling SPI_fnumber during each step of the cursor batch
 * processing.
 * FIXME: This is tricky since the following query only makes sense when the
 * given parameter is a _collection_ uri. Otherwise we might get false
 * positives.
 */

#define QUERY_FIND_SELF_OR_CHILDREN \
          "SELECT uri FROM facts WHERE uri LIKE $1||'%' "


PG_FUNCTION_INFO_V1(facts_assert);
Datum
facts_assert(PG_FUNCTION_ARGS)
{
  Datum           values[4];
  static pg_plan  find_plan, insert_plan, update_plan ;
  static Oid      ctypes[4] = {TEXTOID, TEXTOID, TEXTOID, TEXTOID};
  int             res = -1;

  /* Check for not-NULL */
  if(PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||PG_ARGISNULL(3)) {
    elog(ERROR, "function assert(): Can not assert NULL knowlege");
  }

  values[0] = PG_GETARG_DATUM(0);
  values[1] = PG_GETARG_DATUM(1);
  values[2] = PG_GETARG_DATUM(2);
  values[3] = PG_GETARG_DATUM(3);

  if(SPI_connect() != SPI_OK_CONNECT)
    {
      elog(ERROR, "function facts: SPI_connect failed");
    }

  if(!find_plan)
    {
      pg_plan pl;

      pl = SPI_prepare(QUERY_FIND, 4, ctypes);
      if (pl == NULL) {
	elog(ERROR, "function facts: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
#ifndef NDEBUG
      elog(NOTICE, "Saved query_find plan for function assert()");
#endif
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
#ifndef NDEBUG
      elog(NOTICE, "Saved query_insert plan for function assert()");
#endif
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
#ifndef NDEBUG
      elog(NOTICE, "Saved query_update plan for function assert()");
#endif
      update_plan = SPI_saveplan(pl);
    }

  /* We have a connection to the server core with our own memory context
   * and a precompiled plan - let's see whether we allready have a similar
   * fact in the database.
   */

  res = SPI_execute_plan(find_plan, values, "____", true, 0);
  if (res != SPI_OK_SELECT) {
    elog(ERROR, "function facts: could not check fact. SPI_exep returned %d", res);
    goto FINISH;
  }

  /* Check the number of tuples found */
  switch(SPI_processed) {
  case 0:   /* Nothing similar found -> insert new fact */
    res = SPI_execp(insert_plan, values, "____", 0);
#ifndef NDEBUG
    elog(NOTICE, "Inserting fact ...");
#endif
    goto FINISH;
    break;
  case 1:   /* Similar fact found -> update found fact */
    res = SPI_execp(update_plan, values, "____", 0);
#ifndef NDEBUG
    elog(NOTICE, "Updating fact ...");
#endif
    goto FINISH;
    break;
  default: /* More than one fact? Gosh! */
    elog(ERROR, "function facts: multiple facts found (%d). Please repair.", SPI_processed);
    goto FINISH;
  }

 FINISH:
  SPI_finish();
  PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(facts_forget);
Datum
facts_forget(PG_FUNCTION_ARGS)
{
  Datum           values[3];
  static pg_plan  delete_plan ;
  static Oid      ctypes[3] = {TEXTOID, TEXTOID, TEXTOID};
  int             res = -1;

  if(PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {
    elog(ERROR, "function forget(): Can not forget NULL knowlege");
  }

  values[0] = PG_GETARG_DATUM(0);
  values[1] = PG_GETARG_DATUM(1);
  values[2] = PG_GETARG_DATUM(2);

  if(SPI_connect() != SPI_OK_CONNECT)
    {
      elog(ERROR, "function facts: SPI_connect failed");
    }

  if(!delete_plan)
    {
      pg_plan pl;

      pl = SPI_prepare(QUERY_FORGET, 4, ctypes);
      if (pl == NULL) {
	elog(ERROR, "function facts: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
#ifndef NDEBUG
      elog(NOTICE, "Saved query_delete plan for function assert()");
#endif
      delete_plan = SPI_saveplan(pl);
    }

  res = SPI_execp(delete_plan, values, "___", 0);
  if (res != SPI_OK_DELETE) {
    elog(ERROR, "function facts: could not forget fact. SPI_exep returned %d", res);
    goto FINISH;
  }

 FINISH:
  SPI_finish();
  PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(facts_delete_resource);
Datum
facts_delete_resource(PG_FUNCTION_ARGS)
{
  Datum           values[1];
  text           *uri;
  static pg_plan  delete_resource_plan, delete_collection_plan;
  static Oid      ctypes[1] = {TEXTOID};
  int             res = -1;

  if(PG_ARGISNULL(0)) {
      elog(ERROR, "function delete_resource(): Can not remove NULL resource");
      goto FINISH;
  }

  if(SPI_connect() != SPI_OK_CONNECT)
  {
      elog(ERROR, "function facts:remove_Resource: SPI_connect failed");
      goto FINISH;
  }

  if(!delete_resource_plan)
  {
      pg_plan pl;

      pl = SPI_prepare(QUERY_DELETE_RESOURCE, 1, ctypes);
      if (pl == NULL) {
          elog(ERROR, "function facts:delete_resource(): SPI_saveplan returned %d", SPI_result);
          goto FINISH;
      }
#ifndef NDEBUG
      elog(NOTICE, "Saved query_delete plan for function delete_resource()");
#endif
      delete_resource_plan = SPI_saveplan(pl);
  }

  if(!delete_collection_plan)
  {
      pg_plan pl;

      pl = SPI_prepare(QUERY_DELETE_COLLECTION, 1, ctypes);
      if (pl == NULL) {
          elog(ERROR, "function facts:delete_collection(): SPI_saveplan returned %d", SPI_result);
          goto FINISH;
      }
#ifndef NDEBUG
      elog(NOTICE, "Saved query_delete plan for function delete_collection()");
#endif
      delete_collection_plan = SPI_saveplan(pl);
  }

  values[0] = PG_GETARG_DATUM(0);
  uri = PG_GETARG_TEXT_P(0);

  /* Test for resource/collection-ness NOTA BENE: here we depend on the
   * resource ID (URI) of collections to end with a forward slash. AFAIK this
   * can be safely assumed at this poin.
   */

  {
      char* tvalue;
      size_t tv_length;

      tvalue    = (char *) VARDATA(uri);
      tv_length = VARSIZE(uri) - VARHDRSZ;
      if (tvalue[tv_length-1] == '/')
      {
          res = SPI_execp(delete_collection_plan, values, "____", 0);
      }
      else  /* it's a simple resource */
      {
          res = SPI_execp(delete_resource_plan, values, "____", 0);
      }
  }

  res = SPI_execp(delete_resource_plan, values, "____", 0);
  if (res != SPI_OK_DELETE) {
      switch (res) {
      case SPI_ERROR_ARGUMENT:
          elog(ERROR, "function facts: could not check delete facts for resource. No plan or wrong count!");
          break;
      case SPI_ERROR_PARAM:
          elog(ERROR, "function facts: could not check delete facts for resource. No value given!");
          break;
      default:
          elog(ERROR, "function facts: could not check delete facts for resource. Unknown error (%d)", res);
          break;
      }
    goto FINISH;
  } else {
#ifndef NDEBUG
      elog(NOTICE, "function delete_resource(): deleted %d facts for resource.", SPI_processed);
#endif
  }


  FINISH:
  SPI_finish();
  PG_RETURN_NULL();
}


/* Move a resource (and all it's children if the resource is a collection */
extern Datum facts_move_all(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(facts_move_all);
Datum
facts_move_all(PG_FUNCTION_ARGS)
{
  Portal          cursor;
  Datum           values[2];
  static pg_plan  pos_plan ;
  static Oid      ctypes[1] = {TEXTOID};

  if(PG_ARGISNULL(0))
     elog(ERROR, "function move(): Can not move from NULL uri");

  if (PG_ARGISNULL(1))
    elog(ERROR, "function move(): Can not move to NULL uri");

  if(SPI_connect() != SPI_OK_CONNECT)
    {
      elog(ERROR, "function facts: SPI_connect failed");
    }

  if(!pos_plan)
    {
      /* FIXME: what's the second param for */
      pos_plan = SPI_prepare(QUERY_FIND_SELF_OR_CHILDREN, 1, ctypes);
      if (pos_plan == NULL) {
	elog(ERROR, "function move: SPI_saveplan returned %d", SPI_result);
	goto FINISH;
      }
#ifndef NDEBUG
      elog(NOTICE, "Saved query_delete plan for function move(src, dest)");
#endif
      pos_plan = SPI_saveplan(pos_plan);
    }

  values[0] = PG_GETARG_DATUM(0);
  values[1] = PG_GETARG_DATUM(1);

  /* FIXME: API change! */
  cursor = SPI_cursor_open("RALFS", pos_plan, values, NULL, 1);
  if(cursor)
    {
      TupleDesc tupdesc;
      SPITupleTable *tuptable;
      char buf[8192];
      char *uri;
      int  proc, i;

      do
	{
	  SPI_cursor_fetch(cursor, 1, BATCHSIZE);
	  proc     = SPI_processed;
	  if (proc)
	    {
	      tupdesc  = SPI_tuptable->tupdesc;
	      tuptable = SPI_tuptable;

	      for (i = 0; i < proc; i++)
		{
		  HeapTuple tuple = tuptable->vals[i];
		  uri = SPI_getvalue(tuple, tupdesc, 1);
		  sprintf(buf, "moving '%s'", uri);
		  if(uri)
		    pfree(uri);
#ifndef NDEBUG
		  elog (NOTICE, "EXECQ: %s", buf);
#endif
		}
	    }
	} while ( proc > 0);
      SPI_cursor_close(cursor);
    }
  else
    {
      elog(ERROR, "Couldn't open cursor to move resources");
      goto FINISH;
    }


 FINISH:
  SPI_finish();
  PG_RETURN_NULL();
}


/* is_parent_or_self: uri, uri -> bool
 *
 */
extern Datum is_parent_or_self(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(is_parent_or_self);
Datum
is_parent_or_self(PG_FUNCTION_ARGS)
{
  bool    iscoll;
  bool    res;
  char   *uri;
  char   *self;
  char   *last;

  uri  = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(PG_GETARG_TEXT_P(0))));
  self = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(PG_GETARG_TEXT_P(1))));

  iscoll = PG_GETARG_BOOL(2);

  res = 0;

  if(('/' != *uri) || ('/' != *self))
    {
      elog(ERROR, "function is_parent_or_self expects absolute URIs");
      res = 0;
      goto OUT;
    }

  for(;*uri && *self; uri++, self++)
    {
      last = uri;
      if(*uri != *self)
	{
	  res = 0;
	  break;
	}
    }
  /* We are at the end of uri. Now, thanks to braindead mod_dav,
   * we need to have a look at 'self'.
   * (case *self
   *   '/'  => #t
   *   '0'  => #t
   *   else =4> +#f)
   * Buuut ... if the last char of 'uri' allready was a '/' this
   * will fail! So we need to test for the last seen character of
   * uri as well in the default branch of the switch.
   */
  switch(*self)
    {
    case '/':
    case '\0':
      res = 1;
      break;
    default:
      if('/' == *last)
	res = 1;
      else res = 0;
      break;
    }
 OUT:
  PG_RETURN_BOOL(res);
}
