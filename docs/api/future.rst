
Future commands that aren't yet implemented
===========================================



dbResultError result
~~~~~~~~~~~~~~~~~~~~

Returns error as string if an error occurred while querying. Returns nil if there is no error. (Should it return empty string instead?)

:result: ``<RESULT>`` - The result

Returns: ``<STRING>``


dbResultErrorNum result
~~~~~~~~~~~~~~~~~~~~~~~

Returns error code if there is one. Returns 0 if there is none.

:result: ``<RESULT>`` - The result

Returns: ``<NUMBER>``

dbResultIsError result
~~~~~~~~~~~~~~~~~~~~~~

Checks if a error occured in the query.

:result: ``<RESULT>`` - The result

Returns: ``<BOOL>``



connection dbConnectionEnableThrow bool
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Makes dbExecuteQuery and dbWaitForResult throw SQF Exceptions that can be caught using https://community.bistudio.com/wiki/catch

:connection: ``<DBCONNECTION>`` - A connection
:bool: ``<BOOL>`` - throwing enabled or disabled

Returns: ``<NOTHING>``


query dbBindNamedValue [name, value]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This command modifies the value in `query`. If you want to keep the old query intact you need to :ref:`dbCopyQuery <dbCopyQuery>` first.

:query: ``<QUERY>``
:name: ``<STRING>`` - Name of the value to bind
:value: ``<STRING>`` OR ``<NUMBER>`` OR ``<BOOL>`` - Value to bind to the next unbound `<name>` in the query

Returns: ``<NOTHING>``


| Example: ``SELECT <value> FROM <table>;``
| dbBindNamedValue ["value", "onions"];
| dbBindNamedValue ["table", "shoppinglist"];
| -> ``SELECT onions FROM shoppinglist``

| Maybe other syntax would be better? ``$name``? ``:name`` ?

| ``:name`` seems to be standard elsewhere 
| https://www.php.net/manual/de/pdostatement.bindparam.php
| https://www.javaworld.com/article/2077706/named-parameters-for-preparedstatement.html
| https://docs.oracle.com/cd/B10501_01/appdev.920/a96584/oci05bnd.htm
| https://www.sqlite.org/c3ref/bind_blob.html