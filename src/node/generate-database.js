import fs from 'fs';
import async from 'async';
import r from 'rethinkdb';

import config from './config';

export default function (final_callback) {
  async.waterfall([
    function connect(callback) {
      r.connect(config.rethinkdb, callback);
    },
    function createDatabase(connection, callback) {
      //Create the database if needed.
      r.dbList().contains(config.rethinkdb.db).do(function(containsDb) {
        return r.branch(
          containsDb, {
            created: 0
          },
          r.dbCreate(config.rethinkdb.db)
        );
      }).run(connection, function(err) {
        callback(err, connection);
      });
    },
    function createTable(connection, callback) {
      //Create the table if needed.
      r.tableList().contains('todos').do(function(containsTable) {
        return r.branch(
          containsTable, {
            created: 0
          },
          r.tableCreate('todos')
        );
      }).run(connection, function(err) {
        callback(err, connection);
      });
    },
    function createIndex(connection, callback) {
      //Create the index if needed.
      r.table('todos').indexList().contains('createdAt').do(function(hasIndex) {
        return r.branch(
          hasIndex, {
            created: 0
          },
          r.table('todos').indexCreate('createdAt')
        );
      }).run(connection, function(err) {
        callback(err, connection);
      });
    },
    function waitForIndex(connection, callback) {
      //Wait for the index to be ready.
      r.table('todos').indexWait('createdAt').run(connection, function(err, result) {
        callback(err, connection);
      });
    }
  ], function(err, connection) {
    if (err) {
      console.error(err);
      process.exit(1);
      return;
    }

    final_callback(null);
  });
};