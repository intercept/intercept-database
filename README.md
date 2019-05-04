# what

use MariaDB / MySQL in Arma

# how

define MariaDB or MySQL connection parameters by creating the path & file `@InterceptDB\config.yaml` *in the Arma3 game  directory*:

```yaml
accounts: 
  foo:
    ip: 127.0.0.1
    username: us3r
    password: s3cr3t
    database: foo 
statements:
  foostmt: "select 1 where ? > 42"
```

and execute/retrieve results like this:

```sqf
private _connection = dbCreateConnection "foo"; 
private _query = dbPrepareQueryConfig ["foostmt", [43]]; 
private _result = _connection dbExecute _query; 
private _resultArray = dbResultToArray _result;
systemChat format ["xxx %1", _resultArray];
diag_log _resultArray;

```

**for further documentation see the [Wiki](https://github.com/intercept/intercept-database/wiki)**
