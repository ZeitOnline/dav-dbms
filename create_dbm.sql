-- Filespec: $Id$
-- 
-- $$LICENCE$$
-- This script generates the database structures to
-- handle the CMS resource database.
--
-- WARNING: executing this script will destroy any
-- previously created data!
--
-- Install: as database owner:
--   psql -E -e -f create_dbm.sql template1
--
-- Version: 

-----------------------------------------------------------------[ BASE DATABASE ]--

-- Might fail
DROP DATABASE cms;
COMMIT;

-- Create new database
CREATE DATABASE cms;
COMMIT;

-- Connect to the cms database
\c cms;


-- We need PL/SQL (this might fail in case the PL/SQL
-- language is allready installed).

CREATE FUNCTION "plpgsql_call_handler" () 
  RETURNS opaque 
  AS '/usr/lib/postgresql/lib/plpgsql.so', 'plpgsql_call_handler' 
  LANGUAGE 'C';

CREATE LANGUAGE plpgsql
  HANDLER plpgsql_call_handler;


------------------------------------------------------------------------[ TABLES ]--

-- Our facts table
CREATE TABLE "facts" (
	"uri"       text,
	"namespace" text,
	"name"      text,
	"value"     text   );

---------------------------------------------------------------------[ FUNCTIONS ]--

-- function assert(uri, namespace, name, value) -> bool moved to file 'facts.c'
-- and is implemented in 'C'.


-- forget(uri, namespace, name, value) -> bool
-- Forget a fact about a resource. The return value indicates
-- whether a fact was indeed known to the dbm.
 
DROP FUNCTION forget(text, text, text, text);

CREATE FUNCTION forget(text, text, text, text) RETURNS bool AS'
  DECLARE 
    p_uri       ALIAS FOR $1;
    p_namespace ALIAS FOR $2;
    p_name      ALIAS FOR $3;
    p_value     ALIAS FOR $4;
    result      BOOLEAN  := ''T'';

  BEGIN
    DELETE FROM facts
      WHERE uri       = p_uri AND
            namespace = p_namespace AND
            name      = p_name AND
            value     = p_value;
    -- FIXME: how to test for successfull deletition
    IF NOT FOUND THEN 
      result := ''F'';
    END IF;
    RETURN result;
  END;
' LANGUAGE 'plpgsql';
