var path = require('path');
var fs = require('fs');
var exec = require('child_process').exec;

var rootPath = path.resolve(__dirname, '..');
var pidFilePath = path.resolve(rootPath, 'build/server.pid');
var rethinkdbPIDFilePath = path.resolve(rootPath, 'build/rethinkdb.pid');

fs.exists(pidFilePath, (exists) => {
	if (exists) {
		fs.readFile(pidFilePath, (err, data) => {
			if (err) throw err;
			var pid = parseInt(data);
			exec('taskkill /f /pid ' + pid, function(error, stdout, stderr) {
			});
		});
	}
});

fs.exists(rethinkdbPIDFilePath, (exists) => {
	if (exists) {
		fs.readFile(rethinkdbPIDFilePath, (err, data) => {
			if (err) throw err;
			var pid = parseInt(data);
			exec('taskkill /f /pid ' + pid, function(error, stdout, stderr) {
			});
		});
	}
});