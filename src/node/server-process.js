var path = require('path');
var fs = require('fs');

// Output PID to file
var path_pidFile = path.resolve(g_config.path_root, 'build/server.pid');
fs.writeFile(path_pidFile, process.pid, (err) => { if (err) throw err; });

process.on('SIGHUP', () => {
	process.exit(0);
});
process.on('SIGINT', () => {
	process.exit(0);
});
process.on('SIGTERM', () => {
	process.exit(0);
});