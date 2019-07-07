Possible Errors
===============

| Here are the possible errors that InterceptDB can throw.
| Errors are printed to the logfile and can be caught using the :ref:`dbAddErrorHandler <dbAddErrorHandler>` eventhandler.
| 
| Database server errors for Mysql can be found here: https://dev.mysql.com/doc/refman/8.0/en/server-error-reference.html
| InterceptDB specific errors are listed below


Invalid number of bind values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The number of provided bindValues doesn't match the number of required bindValues, see :ref:`dbBindValueArray <dbBindValueArray>`

:errorID: 2
:errorText: "Invalid number of bind values. Expected {number} got {number}"


Unsupported bind value type
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A unsupported value was bound as a value, see :ref:`dbBindValueArray <dbBindValueArray>`

:errorID: 3
:errorText: "Unsupported bind value type. Got {typeName} on index {index} with value {str bindValue}"
