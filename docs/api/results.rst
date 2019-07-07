===============
Getting results
===============

Getting result data
===================


dbResultAffectedRows result
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns number of affected rows. woah.  

result: ``<RESULT>`` - The result  

Returns: ``<NUMBER>``  


dbResultLastInsertId result
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns last insert id. woah.  

:result: ``<RESULT>`` - The result  

Returns: ``<NUMBER>``  

dbResultToArray result
~~~~~~~~~~~~~~~~~~~~~~

Turns the result set into an array of rows.  

Like this [row1,row2,row3];  

Each row being an array made up of the values in that returned row.  

row1 = [value1, value2, value3]  

values can be of type NUMBER, STRING, BOOL, DBNULL (null values from the database will be returned as :ref:`dbNull <dbNull>`)

:result: ``<RESULT>`` - The result  

Returns: ``<ARRAY>``  


dbResultToParsedArray result
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| Like dbResultToArray. But tries to parse all string values from the database.  
| Turns ``"true"`` into ``true``  
| Turns ``"[1,2,3,4]"`` into ``[1,2,3,4]``  
| Turns ``"123"`` into ``123``  
| If string starts with ``[`` it get's put through parseSimpleArray  
| If string starts with ``t``/``f``/``T``/``F``/number it get's wrapped in [] and put through parseSimpleArray
| Anything else is invalid

:result: ``<RESULT>`` - The result  

Returns: ``<ARRAY>``  



Handling Async results
======================


result dbBindCallback [code, (arguments)]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Code will be called with ``_this = [<RESULT>, arguments]``  

:result: ``<ASYNC_RESULT>`` - Value returned by dbExecuteAsync  
:code: ``<CODE>`` - Script to execute once the results are ready  
:arguments: ``<ANY>`` - Arguments passed to the code.  

Returns: ``<NOTHING>``  


Example:

::

    _result dbBindCallback [{
        params ["_result", "_args"];
        //_Args=1
        DB_RES = [dbResultToArray _result, _args]; 
        systemChat "got result!";
    }, 1];


dbWaitForResult result
~~~~~~~~~~~~~~~~~~~~~~

| Does exactly what you think it does. But also freezes the game even in scheduled! (to be changed in future updates)  
| Essentially converts a ASYNC_RESULT into a normal RESULT  

:result: ``<ASYNC_RESULT>`` - Value returned by dbExecuteAsync  

Returns: ``<RESULT>``

Example: ``_result = dbWaitForResult _asyncResult``;  

.. _dbNull:

dbNull
~~~~~~

| Returns a dbNull value (just like ``objNull`` or other Arma null values)

Returns: ``<DBNULL>``

dbNull type can be configured in the :doc:`config file </intro/config-file>` to compare equal to empty string

DBNullEqualEmptyString set to ``true``
::
    dbNull == "" //true
    "" == dbNull //true
    isNull dbNull //true

DBNullEqualEmptyString set to ``false``
::
    dbNull == "" //false
    "" == dbNull //false
    isNull dbNull //true

