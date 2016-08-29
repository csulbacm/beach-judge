const DO_MINIFY = false;

import path from 'path';
import fs from 'fs';
import uglifyjs from 'uglify-js';

import config from './config';

const uglifyjs_options = {
};

if (global.g_files == null) global.g_files = {};

function render(target)
{
  var key = `/${target}.min.js`;
  var filepath = path.resolve(config.path_root, `src/js/${target}`);
  var files = fs.readdirSync(filepath);
	for (var a in files)
		files[a] = path.resolve(filepath, files[a]);
	if (DO_MINIFY) {
	  g_files[key] = uglifyjs.minify(files, uglifyjs_options).code;
	} else {
		var buffer = '';
		for (var a in files) {
			buffer += fs.readFileSync(files[a], 'utf8');
			if (buffer.charAt(buffer.length - 1) !== '\n')
				buffer += '\n';
		}
		g_files[key] = buffer;
	}
}

const targets = [
   'admin',
   'common',
   'judge',
   'public'
];

for (var target in targets)
  render(targets[target]);