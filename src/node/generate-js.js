const DO_MINIFY = false;

var path = require('path');
var fs = require('fs');
var uglifyjs = require('uglify-js');

var uglifyjs_options = {
};

if (global.g_files == null) global.g_files = {};

function minify(filepath)
{
	var files = fs.readdirSync(filepath);
	for (var a in files)
		files[a] = path.resolve(filepath, files[a]);

	if (DO_MINIFY) {
		return uglifyjs.minify(files, uglifyjs_options).code;
	} else {
		var buffer = '';
		for (var a in files) {
			buffer += fs.readFileSync(files[a], 'utf8');
			if (buffer.charAt(buffer.length - 1) !== '\n')
				buffer += '\n';
		}
		return buffer;
	}
}

g_files['/admin.min.js'] = minify(path.resolve(g_config.path_root, 'src/js/admin'));
g_files['/common.min.js'] = minify(path.resolve(g_config.path_root, 'src/js/common'));
g_files['/judge.min.js'] = minify(path.resolve(g_config.path_root, 'src/js/judge'));
g_files['/public.min.js'] = minify(path.resolve(g_config.path_root, 'src/js/public'));