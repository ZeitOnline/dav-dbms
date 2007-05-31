-- Filespec: $Id$

SET search_path = public;
SET autocommit TO 'on';

-- Seems that (at least some versions) of PostgreSQL can
-- rather cope with `type stubs' than with `func stubs',
-- so it's better to get the funcs first, then the types:

CREATE OR REPLACE FUNCTION facts_uri_in(opaque)
  RETURNS facts_uri
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_uri_in'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION facts_uri_out(opaque)
  RETURNS cstring
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_uri_out'
  LANGUAGE 'C';

 -- Typecasts
CREATE TYPE facts_uri (
  INTERNALLENGTH = VARIABLE,
  EXTERNALLENGTH = VARIABLE,
  INPUT  = facts_uri_in,
  OUTPUT = facts_uri_out

);

COMMENT ON TYPE facts_uri IS
 'This datatype holds the URIs used as document identifiers within the facts database';

-- NOTE Doesn't work. Real Casts (TM) are coming
-- DROP CAST (text AS facts_uri);
-- CREATE CAST (text AS facts_uri) WITH FUNCTION facts_uri_in(opaque) AS IMPLICIT;


 -- Comparison/Indexing
CREATE OR REPLACE FUNCTION facts_uri_eq(facts_uri, facts_uri)
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_uri_eq'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION facts_uri_is_parent_or_self(facts_uri, facts_uri)
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'is_parent_or_self'
  LANGUAGE 'C';

CREATE OPERATOR = (
  LEFTARG    = facts_uri, 
  RIGHTARG   = facts_uri, 
  PROCEDURE  = facts_uri_eq,
  COMMUTATOR = '=',
  NEGATOR = '<>',
  HASHES
 -- RESTRICT = eqsel, JOIN = eqjoinsel,
 -- SORT1 = '<', SORT2 = '<'
);

CREATE OPERATOR <> (
  LEFTARG    = facts_uri, 
  RIGHTARG   = facts_uri, 
  PROCEDURE  = facts_uri_neq,
  COMMUTATOR = '<>',
  NEGATOR    = '='
 -- RESTRICT = eqsel, JOIN = eqjoinsel,
 -- SORT1 = '<', SORT2 = '<'
);

CREATE OPERATOR ~~ (
  LEFTARG    = facts_uri, 
  RIGHTARG   = facts_uri, 
  PROCEDURE  = facts_uri_is_parent_or_self
--  COMMUTATOR = '<>',
--  NEGATOR    = '='
 -- RESTRICT = eqsel, JOIN = eqjoinsel,
 -- SORT1 = '<', SORT2 = '<'
);


CREATE OR REPLACE FUNCTION assert(text, text, text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_assert'
  LANGUAGE 'C';


CREATE OR REPLACE FUNCTION forget(text, text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_forget'
  LANGUAGE 'C';

 
CREATE OR REPLACE FUNCTION move(text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'facts_move_all'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION is_parent_or_self(text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/8.2/lib/facts.so', 'is_parent_or_self'
  LANGUAGE 'C'
  WITH (iscachable);

