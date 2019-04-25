# what

use MariaDB / MySQL in Arma

# how

define MariaDB or MySQL connection parameters by editing `config.yaml` in the mod directory:

```yaml
accounts: 
  foo:
    ip: 127.0.0.1
    username: us3r
    password: s3cr3t
    database: foo 
statements:
  foostmt: "select 1"
```

and execute/retrieve results like this:

```sqf
private _connection = dbCreateConnection "foo"; 
private _query = dbPrepareQuery "foostmt"; 
private _result = _connection dbExecute _query; 
private _resultArray = dbResultToArray _result;
systemChat format ["xxx %1", _resultArray];
diag_log _resultArray;

```
