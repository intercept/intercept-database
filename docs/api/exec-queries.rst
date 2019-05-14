Executing Queries
=================




connection dbExecute query
~~~~~~~~~~~~~~~~~~~~~~~~~~

| This function behaves differently in scheduled and unscheduled.
| Scheduled: Suspends the script like a sleep/waitUntil would do, and continues once result is ready.
| Unscheduled: Freezes the game until the result is ready. (You probably want to use `dbExecuteAsync`)


:connection: ``<DBConnection>`` - The connection to execute the query on
:query: ``<QUERY>`` - the query object returned by dbPrepareQuery

Returns: ``<RESULT>``


connection dbExecuteAsync query
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| This function executes the query in a seperate thread and returns a handle to the task.
| You can bind callbacks to it, or wait on the task to finish (see below)

:connection: ``<DBConnection>`` - The connection to execute the query on
:query: ``<QUERY>`` - the query object returned by dbPrepareQuery

Returns: ``<ASYNC_RESULT>`` (See :doc:`results: Handling Async results`)