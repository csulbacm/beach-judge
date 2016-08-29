var path = require('path');
var config = require(path.resolve(__dirname, 'config.js'));
require.main.paths.push(path.resolve(g_config.path_root, 'build/external/nodejs/node_modules'));

//console.log(require('less'));
//console.log(require('uglify-js'));
var fs = require('fs');
var async = require('async');
var r = require('rethinkdb');


require('./generate-js.js');

async.waterfall([
  function connect(callback) {
  	r.connect(g_config.rethinkdb, callback);
  },
  function createDatabase(connection, callback) {
  	//Create the database if needed.
  	r.dbList().contains(g_config.rethinkdb.db).do(function (containsDb) {
  		return r.branch(
		  containsDb,
		  { created: 0 },
		  r.dbCreate(g_config.rethinkdb.db)
		);
  	}).run(connection, function (err) {
  		callback(err, connection);
  	});
  },
  function createTable(connection, callback) {
  	//Create the table if needed.
  	r.tableList().contains('todos').do(function (containsTable) {
  		return r.branch(
		  containsTable,
		  { created: 0 },
		  r.tableCreate('todos')
		);
  	}).run(connection, function (err) {
  		callback(err, connection);
  	});
  },
  function createIndex(connection, callback) {
  	//Create the index if needed.
  	r.table('todos').indexList().contains('createdAt').do(function (hasIndex) {
  		return r.branch(
		  hasIndex,
		  { created: 0 },
		  r.table('todos').indexCreate('createdAt')
		);
  	}).run(connection, function (err) {
  		callback(err, connection);
  	});
  },
  function waitForIndex(connection, callback) {
  	//Wait for the index to be ready.
  	r.table('todos').indexWait('createdAt').run(connection, function (err, result) {
  		callback(err, connection);
  	});
  }
], function (err, connection) {
	if (err) {
		console.error(err);
		process.exit(1);
		return;
	
	}

	require('./server-process.js');
	require('./generate-css.js');
	require('./generate-js.js');
	require('./generate-html.js');
	require('./http.js');

//	startExpress(connection);
});