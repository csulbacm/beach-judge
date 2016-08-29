var url = require('url');
var qs = require('querystring');
var path = require('path');
var express = require('express');
var bodyParser = require('body-parser');

import config from './config';

var app = express();

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: true
}));

app.listen(config.port_html, function() {
  console.log('Starting Express on Port ' + config.port_html + '...');
});

// app.get('/*', function(req, res) {
//   console.log('GET: ' + req.url);
//   if (g_pages[req.url] != null) {
//     res.end(g_pages[req.url]);
//   }
// 	else if (g_files[req.url] != null) {
//     res.end(g_files[req.url]);
//   }
// 	else {
//     // TODO: 404
//     res.end('404');
//   }
// });
//
// app.post('/login', function(req, res) {
//   console.log('POST: ' + req.url);
//   console.log(req.body);
// });

export default app;
