
\c cms


CREATE OR REPLACE FUNCTION assert(text, text, text, text) 
  RETURNS bool
  AS '/home/ralf/projects/ZEIT/CMS-DBM/facts.so', 'facts_assert'
  LANGUAGE 'C';


CREATE OR REPLACE FUNCTION forget(text, text, text) 
  RETURNS bool
  AS '/home/ralf/projects/ZEIT/CMS-DBM/facts.so', 'facts_forget'
  LANGUAGE 'C';