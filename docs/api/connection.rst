Connection commands
===================

dbCreateConnection configName
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creates a connection based on details in the :doc:`config file </intro/config-file>` in ``accounts.<configName>``

.. note::
        Connection is not established until the first query.

:configName: ``<STRING>`` - The config name of the connection

.. attention::
    configName is case-sensitive

Returns: <DBConnection>


dbCreateConnection [ip, port, user, pw, db]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creates a connection.

.. note::
        Connection is not established until the first query.

:ip: ``<STRING>`` - the IP Address or Domain of the database server
:port: ``<NUMBER>`` - the port of the database server (usually 3306)
:user: ``<STRING>`` - the user to log in with
:pw: ``<STRING>`` - the password (duh)
:db: ``<STRING>`` - the database to use (Equal to `use <db>` SQL command)

Returns: <DBConnection>



dbIsConnected Connection
~~~~~~~~~~~~~~~~~~~~~~~~

| Returns whether the connection is currently connected to the database server.
| Also checks if a worker thread is connected.

:connection: ``<DBCONNECTION>`` - A connection

Returns: ``<BOOL>``





dbPing connection 
~~~~~~~~~~~~~~~~~

| Executes a ``SELECT 1;`` on the database server and returns true if it get's 1 back. Returns false on error.
| Suspends in scheduled, freezes in unscheduled.
| (Should this return the actual error string somehow?, Should this call error handlers?)

:connection: ``<DBCONNECTION>`` - A connection

Returns: ``<BOOL>``



.. _dbAddErrorHandler:

connection dbAddErrorHandler code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| Registers a global error handler on the connection, if any query on the connection causes an error, that function will be called with ``_this = [errorString, errorCode, query]``.
| There can be multiple error handlers, they will be executed from first to last added.
| If one of the error handlers returns ``true`` the error will be considered handled and the other handlers won't be called.
| If error handlers are present, errors won't be printed to RPT.
| Example _this:
| ``["Lost connection to MySQL server at 'reading authorization packet', system error: 10061",2013,"testQuery5"]``
| ``["You have an error in your SQL syntax; check the manual that corresponds to your MariaDB server version for the right syntax to use near 'testQuery5' at line 1",1064,"testQuery5"]``
| ``["Unknown column 'none' in 'field list'",1054,"SELECT none"]``
| #TODO add the query config name to _this too. 
| Error codes are explained on :doc:`config file </api/errors>`

:connection: ``<DBCONNECTION>`` - A connection
:code: ``<CODE>`` - Script code. 

Returns: ``<NOTHING>``

 

connection dbLoadSchema schemaName
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Executes a SQL file. Path is defined in config.

:connection: ``<DBCONNECTION>`` - A connection
:schemaName: ``<STRING>`` - schema name from config. 

.. attention::
    schemaName is case-sensitive

Returns: ``<NOTHING>``

