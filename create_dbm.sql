-- $$LICENCE$$
-- This script generates the database structures to
-- handle the CMS resource database.
--
-- WARNING: executing this script will destroy any
-- previously created data!
--
-- Install: psql -f create_dbm.sql themplate1
--
-- Version: 

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

-- Our facts table
CREATE TABLE "facts" (
	"uri"       text,
	"namespace" text,
	"name"      text,
	"value"     text   );

-- Functions to handle 'assert'-like queries.

-- assert(uri, namespace, name, value) -> bool
-- Store a fact (uri, namespace, name, value) into the database.
-- If a value for the given (uri, namespace, name) combination allready
-- exists, modify it, otherwise create a new tuple.
-- NOTE: this algorithm assumes that we do not need multi-sets for
--       a given resource. Is this true?

DROP FUNCTION assert(text, text, text, text);

CREATE FUNCTION assert(text, text, text, text) RETURNS bool AS'

  DECLARE 
    p_uri       ALIAS FOR $1;
    p_namespace ALIAS FOR $2;
    p_name      ALIAS FOR $3;
    p_value     ALIAS FOR $4;
    result      BOOLEAN  := ''F'';
    fact        RECORD;

  BEGIN
    -- Test if there exists a fact for (uri, namespace, name)
    SELECT INTO fact *
      FROM facts 
      WHERE uri       = p_uri AND
            namespace = p_namespace AND
            name      = p_name;
    IF NOT FOUND THEN 
      INSERT INTO facts VALUES (p_uri, p_namespace, p_name, p_value);
      result := ''T'';
    ELSE
      UPDATE facts SET value = p_value 
        WHERE uri = p_uri AND namespace = p_namespace AND name = p_name; 
    END IF;
    RETURN result;
  END;
' LANGUAGE 'plpgsql';

-- forget(uri, namespace, name, value) -> bool
-- 
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
