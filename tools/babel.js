var path = require('path');
var path_root = path.resolve(__dirname, '..');
require.main.paths.push(path.resolve(path_root, 'build/external/nodejs/node_modules'));

var fs = require('fs');
var async = require('async');
var babel = require('babel-core');

var options = {
	presets: ['es2015'],

};

var args = process.argv.slice(2);
var files = fs.readdirSync(args[0]);
for (var a in files) {
	var inPath = path.resolve(args[0], files[a]);
	var outPath = path.resolve(args[1], files[a]);
	var str = babel.transform(fs.readFileSync(inPath, 'utf8'), options).code;
	if (files[a] === 'index.js') {
		str = 'var path = require(\'path\');\n\
var config = require(path.resolve(__dirname, \'config.js\'));\n\
require.main.paths.push(path.resolve(g_config.path_root, \'build/external/nodejs/node_modules\'));\n' + str;
	} else {
		str = 'require.cache[__filename].paths = require.cache[__filename].parent.paths;\n' + str;
	}
	fs.writeFileSync(outPath, str, 'utf8');
}