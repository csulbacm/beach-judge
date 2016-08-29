var fs = require('fs');
var path = require('path');
var cp = require('child_process');

// Launch Rethink
{
	var out = fs.openSync(path.resolve(__dirname, '../build/rethinkdb_out.log'), 'a');
	var err = fs.openSync(path.resolve(__dirname, '../build/rethinkdb_err.log'), 'a');
	var exe = path.resolve(__dirname, '../build/external/rethinkdb/rethinkdb.exe');
	var child = cp.spawn(exe, ['--http-port', 8081], { detached: true, stdio: [ 'ignore', 'ignore', 'ignore' ] });

	var pidFile = path.resolve(__dirname, '../build/rethinkdb.pid');
	fs.writeFile(pidFile, child.pid, (err) => { if (err) throw err; });
	child.unref();
}

// Launch beachJudge
{
	var out = fs.openSync(path.resolve(__dirname, '../build/beachjudge_out.log'), 'a');
	var err = fs.openSync(path.resolve(__dirname, '../build/beachjudge_err.log'), 'a');
	var index = path.resolve(__dirname, '../build/node/generated/index.js');
	var exe = path.resolve(__dirname, '../build/external/nodejs/node.exe');

	//var child = cp.spawn('cmd.exe', ['/c', 'start', '/min', path.resolve(__dirname, '../tools/run-server.bat')], { detached: true, stdio: [ 'ignore', 'ignore', 'ignore' ] });
	var child = cp.spawn('cmd.exe', ['/c', 'start', '/min', exe, index], { detached: true, stdio: [ 'ignore', out, err ] });
	// TODO: Capture errors from script execution and display them to this script's stdout stream
	child.unref();
}