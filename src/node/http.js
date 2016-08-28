var url = require('url');
var qs = require('querystring');
var path = require('path');
var express = require(path.resolve(g_config.path_mods, 'express'));
var bodyParser = require(path.resolve(g_config.path_mods, 'body-parser'));

var app = express();
global.g_app = app;

g_app.use(bodyParser.json());
g_app.use(bodyParser.urlencoded({ extended: true }));

g_app.listen(g_config.port_html, function () {
	console.log('Starting Express on Port ' + g_config.port_html + '...');
});

g_app.get('/*', function (req, res) {
	console.log('GET: ' + req.url);
	if (g_pages[req.url] != null) {
		res.end(g_pages[req.url]);
	} else if (g_files[req.url] != null) {
		res.end(g_files[req.url]);
	} else {
		// TODO: 404
		res.end('404');
	}
});

g_app.post('/login', function (req, res) {
	console.log('POST: ' + req.url);
	console.log(req.body);
});