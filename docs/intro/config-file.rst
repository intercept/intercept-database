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
    
    global:
     enableDynamicQueries: true #Allow queries to be created from SQF, if false only statements from config are allowed
    
    schemas:
     test: schema.sql #Filename relative to config.yaml to be used in dbLoadSchema


Config has to be in ``Arma 3/@InterceptDB/config.yaml`` Other subfolders or renaming the @InterceptDB folder doesn't work. Folder name is also case sensitive on linux.

The config is loaded at preInit. If anything on the config loading fails, a error will be printed to the RPT.

#TODO if people need it, add ability to set connection options in config.