The InterceptDB Config file
===========================

.. code:: yaml

    accounts:
     maindb: #production db, don't break things here!
      ip: 127.0.0.1
      username: root
      password: lulz
      database: production
      port: 3306 #optional
     testdb: #testserver
      ip: 127.0.0.2
      username: root
      password: lulz
      database: production
      port: 3306 #optional
      opt_compress: false #set the MYSQL_OPT_COMPRESS option
    
    statements:
     insertStuff: INSERT INTO table (a,b,c) VALUES (?,?,?)
     deleteStuff: DELETE FROM table WHERE a=?
     longQuery: >
      SELECT stuff
      FROM table
      WHERE
      isThisALongQuery=1 AND
      queriesCanBeMultiline=1 AND
      thatsWhyILikeYAML=5;
     queryWithOptions:
      query: SELECT NOW(), tinyIntValue FROM MyTable;
      parseDateType: array #I want this specific statement to return DateTime values in array format
      parseTinyintAsBool: true #I want this specific statement to return my tinyInt as a boolean in dbResultTo(Parsed)Array
    
    global:
     enableDynamicQueries: true #Allow queries to be created from SQF, if false only statements from config are allowed
     parseDateType: string #This is a enum, one of the below values is allowed
      #string: default. Return Date/DateTime as "2018-12-24 13:45:11"
      #stringMS: Return Date/DateTime as "2018-12-24 13:45:11.123"
      #array: Return Date/DateTime as [year,month,day,hour,minute,second,millisecond] (yes both have time too, date will be 0 hours) in dbResultTo(Parsed)Array
      #timestamp: Return Date/DateTime as a timestamp as a number (this can incur precision loss)
      #timestampString: Return Date/DateTime as a unix timestamp in a string
      #timestampStringMS: Return Date/DateTime as a millisecond unix timestamp in a string
     parseTinyintAsBool: false #returns tinyint as bool in dbResultTo(Parsed)Array
     DBNullEqualEmptyString: false #whether dbNull == "" returns true
     logging:
      directory: dbLog #logging directory, relative to arma directory, will be created if it doesn't exist
      querylog: false #log all queries with timestamp
      threadlog: false #log threading activity (high bandwidth log)
    
    schemas:
     test: schema.sql #Filename relative to config.yaml to be used in dbLoadSchema


Config has to be in ``Arma 3/@InterceptDB/config.yaml`` Other subfolders or renaming the @InterceptDB folder doesn't work. Folder name is also case sensitive on linux.

The config is loaded at preInit. If anything on the config loading fails, a error will be printed to the RPT.

Per-statement options take precendence over global options