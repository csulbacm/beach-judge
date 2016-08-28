var path = require('path');
var fs = require('fs');
var pug = require(path.resolve(g_config.path_mods, 'pug'));

var path_pugStaticFiles = path.resolve(g_config.path_root, 'src/pug/static');
var path_pugDynamicFiles = path.resolve(g_config.path_root, 'src/pug/dynamic');

var pug_options = {
	doctype: 'html',
	pretty: true
};

// TODO: Use compile for dynamic and render for static
// TODO: Minify

if (global.g_pages == null) global.g_pages = {};

pug_options.filename = path.resolve(path_pugStaticFiles, 'login.pug');
g_pages['/'] = pug.renderFile(pug_options.filename, pug_options);