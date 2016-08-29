require.cache[__filename].paths = require.cache[__filename].parent.paths;

var path = require('path');
var fs = require('fs');
var less = require('less');

var path_lessFiles = path.resolve(g_config.path_root, 'src/less');
var less_options = {
	paths: [path_lessFiles],
	doctype: 'html',
	filename: 'public.less',
	compress: true
};

if (global.g_files == null) global.g_files = {};

less.render(fs.readFileSync(path.resolve(path_lessFiles, 'public.less'), 'utf8'), less_options, function (e, output) {
	g_files['/public.min.css'] = output.css
});
less.render(fs.readFileSync(path.resolve(path_lessFiles, 'user.less'), 'utf8'), less_options, function (e, output) {
	g_files['/user.min.css'] = output.css
});
less.render(fs.readFileSync(path.resolve(path_lessFiles, 'judge.less'), 'utf8'), less_options, function (e, output) {
	g_files['/judge.min.css'] = output.css
});
less.render(fs.readFileSync(path.resolve(path_lessFiles, 'admin.less'), 'utf8'), less_options, function (e, output) {
	g_files['/admin.min.css'] = output.css
});