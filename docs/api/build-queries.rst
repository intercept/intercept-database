Building Queries
================



dbPrepareQuery query
~~~~~~~~~~~~~~~~~~~~

Prepares a query.

:query: ``<STRING>`` - The SQL Query String

Returns: ``<QUERY>``



dbPrepareQuery [query, bindValues]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepares a query and directly binds some values to it.

:query: ``<STRING>`` - The SQL Query String
:bindValues: ``<ARRAY>`` - List of values to bind to ``?`` in the query string. See :ref:`dbBindValueArray <dbBindValueArray>` for more information.

Returns: ``<QUERY>``

| Example:
| ``dbPrepareQuery ["SELECT ? FROM ? WHERE ?=?", ["data", "table", "value", 5]]``
| -> ``SELECT data FROM table WHERE value=5``


dbPrepareQueryConfig configName
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepares a query based on details in the :doc:`config file </intro/config-file>` in ``statements.<configName>``

:configName: ``<STRING>`` - The config name of the query

.. attention::
    configName is case-sensitive

Returns: ``<QUERY>``


dbPrepareQueryConfig [configName, bindValues]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepares a query based on details in the :doc:`config file </intro/config-file>` in ``statements.<configName>``

:configName: ``<STRING>`` - The config name of the query
:bindValues: ``<ARRAY>`` - List of values to bind to ``?`` in the query string (See above)

.. attention::
    configName is case-sensitive

Returns: ``<QUERY>``


query dbBindValue value
~~~~~~~~~~~~~~~~~~~~~~~
:query: ``<QUERY>``
:value: ``<STRING>`` OR ``<NUMBER>`` OR ``<BOOL>`` OR ``<ARRAY>`` - Value to bind to the next unbound ``?`` in the query

Returns: ``<NOTHING>``

.. note::
    ARRAY values are automatically converted to string. Meaning ``[1,2,3]`` will get bound as ``"[1,2,3]"``

.. warning::
    This command modifies the value in ``query``. If you want to keep the old query intact you need to :ref:`dbCopyQuery <dbCopyQuery>` first.

.. _dbBindValueArray:

query dbBindValueArray [value, value...]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Binds multiple values to the next ``?`` in the query, in same order as the ``?`` occur in the query. 


:query: ``<QUERY>``
:value: ``<STRING>`` OR ``<NUMBER>`` OR ``<BOOL>`` OR ``<ARRAY>`` - Value to bind to the next unbound ``?`` in the query

Returns: ``<NOTHING>``

.. note::
    ARRAY values are automatically converted to string. Meaning ``[1,2,3]`` will get bound as ``"[1,2,3]"``

.. warning::
    This command modifies the value in `query`. If you want to keep the old query intact you need to :ref:`dbCopyQuery <dbCopyQuery>` first.

Example: ``_query = dbPrepareQuery "SELECT ? FROM ? WHERE ?=?"``
``_query dbBindValueArray ["data", "table", "value", 5]``
-> ``SELECT data FROM table WHERE value=5``

dbGetBoundValues query
~~~~~~~~~~~~~~~~~~~~~~

Returns array of all values currently bound to this query

returns ``<ARRAY>``

.. _dbCopyQuery:

dbCopyQuery query
-----------------
query: ``<QUERY>`` - the query object returned by dbPrepareQuery

.. tip::
    There is also the short version ``+ query`` which copies just like with Arrays and Numbers.

Returns: ``<NOTHING>``

| Copies a query with all currently bound values.
| Example: ``_query = dbPrepareQuery "SELECT ? FROM ? WHERE ?=?"``
| ``_query dbBindValueArray ["data", "table"]``
| _query -> ``SELECT data FROM table WHERE ?=?``
| ``_copyOfQuery = dbCopyQuery _query;``
| _copyOfQuery -> ``SELECT data FROM table WHERE ?=?``
| ``_copyOfQuery dbBindValueArray ["value", 5]``
| _copyOfQuery -> ``SELECT data FROM table WHERE value=5``
| _query -> ``SELECT data FROM table WHERE ?=?``
