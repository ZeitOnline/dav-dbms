/* Filespec: $Id$ */

typedef struct 
{
  int32      len;		/* TOAST convention */
  uint16     depth;		/* the resources' level */
  char      *uri;		/* the path as string */
} facts_uri;
