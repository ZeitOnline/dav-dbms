-- Filespec: $Id$
-- Process me with: psql -Ucmsadmin -f < my_filename.sql

-- Namespace and name together build one atomic
-- unit, no need to create individual indices.
-- This leaves us with the question of: why not have
-- one datatype for this entity?

DROP INDEX name_btree_idx;

CREATE INDEX name_btree_idx
  ON facts USING btree (namespace, name);

-- Reverse map a property to a URI:

DROP INDEX prop_val_to_uri;

CREATE INDEX prop_val_to_uri
    ON facts USING btree (namespace, name, value) WHERE namespace <> 'http://namespaces.zeit.de/CMS/tagging'::text;

-- This index speeds up queries for all values for
-- a given URI/property

DROP INDEX all_values_idx;

CREATE INDEX all_values_idx
  ON facts USING btree (uri, namespace, name);

-- Cluster database on index

CLUSTER full_btree_idx on facts;

DROP INDEX events_by_logdate;

CREATE INDEX CONCURRENTLY events_by_logdate
  ON triggers USING btree (logdate) ;
