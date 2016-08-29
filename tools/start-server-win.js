var fs = require('fs');
var path = require('path');
var cp = require('child_process');

var out = fs.openSync('./out.log', 'a');
var err = fs.openSync('./out.log', 'a');
var out2 = fs.openSync('./out3.log', 'a');
var err2 = fs.openSync('./out3.log', 'a');
var nodeExe = path.resolve(__dirname, '../build/external/nodejs/node.exe');
//var index = path.resolve(__dirname, '../src/node/index.js');
var index = path.resolve(__dirname, '../build/node/index.js');
//var rethinkWrapper = path.resolve(__dirname, './wrapper-rethinkdb-win.js');

var rethinkdbExe = path.resolve(__dirname, '../build/external/rethinkdb/rethinkdb.exe');

var rethinkchild = cp.spawn(rethinkdbExe, ['--http-port', 8081], { detached: true, stdio: [ 'ignore', 'ignore', 'ignore' ] });

var path_rethinkdbPIDFile = path.resolve(__dirname, '../build/rethinkdb.pid');
fs.writeFile(path_rethinkdbPIDFile, rethinkchild.pid, (err) => { if (err) throw err; });
rethinkchild.unref();


var child = cp.spawn('cmd.exe', ['/c', 'start', '/min', path.resolve(__dirname, '../tools/run-server.bat')], { detached: true, stdio: [ 'ignore', 'ignore', 'ignore' ] });
//var child = cp.spawn('cmd.exe', ['/c', 'start', '/min', nodeExe, index, ';', 'pause'], { detached: true, stdio: [ 'ignore', 'ignore', 'ignore' ] });
// TODO: Capture errors from script execution and display them to this script's stdout stream
child.unref();