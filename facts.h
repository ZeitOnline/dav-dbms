/* Filespec: $Id$ */

typedef struct
{
  int32 dlen;			/* TOAST convention */
  uint16 depth;			/* the resources' level */
  uint32 len;			/* length of value */
  char val;			/* the path as string */
}
facts_uri;

Datum facts_uri_in (PG_FUNCTION_ARGS);
Datum facts_uri_out (PG_FUNCTION_ARGS);
Datum facts_uri_eq (PG_FUNCTION_ARGS);
Datum facts_uri_neq(PG_FUNCTION_ARGS);
Datum facts_uri_lt (PG_FUNCTION_ARGS);
Datum facts_uri_le (PG_FUNCTION_ARGS);
Datum facts_uri_gt (PG_FUNCTION_ARGS);
Datum facts_uri_ge (PG_FUNCTION_ARGS);
Datum facts_uri2text (PG_FUNCTION_ARGS);
Datum facts_text2uri (PG_FUNCTION_ARGS);

Datum facts_assert(PG_FUNCTION_ARGS);
Datum facts_forget(PG_FUNCTION_ARGS);
Datum facts_delete_resource(PG_FUNCTION_ARGS);
