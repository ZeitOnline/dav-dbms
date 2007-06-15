-- Filespec: $Id$
-- Process me with: psql -Ucmsadmin -f < my_filename.sql
-- Index on property values.

DROP INDEX value_hash_idx;

CREATE INDEX value_hash_idx 
  ON facts USING hash (value);

-- Unfortunately PostgreSQL currently doesn't support
-- hash indices for multiple values.

-- Namespace and name together build one atomic
-- unit, no need to create individual indices.
-- This leaves us with the question of: why not have
-- one datatype for this entity?

DROP INDEX name_btree_idx;

CREATE INDEX name_btree_idx 
  ON facts USING btree (namespace, name);

-- Reverse map a property to a URI:

DROP INDEX full_btree_idx;

create INDEX full_btree_idx
  ON facts USING btree (namespace, name,value);

-- This index speeds up queries for all values for
-- a given URI/property

DROP INDEX all_values_idx;

create INDEX all_values_idx
  ON facts USING btree (uri, namespace, name);
