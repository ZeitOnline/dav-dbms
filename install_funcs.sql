
\c cms


CREATE OR REPLACE FUNCTION assert(text, text, text, text) 
  RETURNS bool
  AS '/usr/local/lib/postgresql/lib/facts.so', 'facts_assert'
  LANGUAGE 'C';


CREATE OR REPLACE FUNCTION forget(text, text, text) 
  RETURNS bool
  AS '/usr/local/lib/postgresql/lib/facts.so', 'facts_forget'
  LANGUAGE 'C';
