-- Filespec: $Id$
\c cms


CREATE TYPE facts_uri (
  INTERNALLENGTH = VARIABLE,
  EXTERNALLENGTH = VARIABLE,
  INPUT  = facts_uri_in,
  OUTPUT = facts_uri_out

);

COMMENT ON TYPE facts_uri IS
 'This datatype holds the URIs used as document identifiers within the facts database';

CREATE OR REPLACE FUNCTION facts_uri_in(opaque)
  RETURNS facts_uri
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_uri_in'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION facts_uri_out(opaque)
  RETURNS facts_uri
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_uri_out'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION facts_uri_eq(facts_uri, facts_uri)
  RETURNS bool
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_uri_eq'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION facts_uri_is_parent_or_self(facts_uri, facts_uri)
  RETURNS bool
  AS '/usr/lib/postgresql/lib/facts.so', 'is_parent_or_self'
  LANGUAGE 'C';

CREATE OPERATOR = (
  LEFTARG    = facts_uri, 
  RIGHTARG   = facts_uri, 
  PROCEDURE  = facts_uri_eq,
  COMMUTATOR = '=',
  NEGATOR = '<>'
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
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_assert'
  LANGUAGE 'C';


CREATE OR REPLACE FUNCTION forget(text, text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_forget'
  LANGUAGE 'C';

 
CREATE OR REPLACE FUNCTION move(text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/lib/facts.so', 'facts_move_all'
  LANGUAGE 'C';

CREATE OR REPLACE FUNCTION is_parent_or_self(text, text) 
  RETURNS bool
  AS '/usr/lib/postgresql/lib/facts.so', 'is_parent_or_self'
  LANGUAGE 'C'
  WITH (iscachable);
