import url from 'url';
import qs from 'querystring';
import path from 'path';
import express from 'express';
import bodyParser from 'body-parser';
import async from 'async';

import config from './config';
import generate_html from './generate-html';
import generate_database from './generate-database';
import generate_css from './generate-css';
import generate_js from './generate-js';

const app = express();

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: true
}));

app.get('/*', function(req, res) {
  console.log('GET: ' + req.url);
  if (g_pages[req.url] != null) {
    res.end(g_pages[req.url]);
  }
	else if (g_files[req.url] != null) {
    res.end(g_files[req.url]);
  }
	else {
    // TODO: 404
    res.end('404');
  }
});

app.post('/login', function(req, res) {
  console.log('POST: ' + req.url);
  console.log(req.body);
});

export default app;

// Startup Process

async.auto({
  init_database: function(callback) {
    generate_database(callback);
  }, 
  start_express: ['init_database', function(results, callback) {
    app.listen(config.port_html, () => {
      console.log(`Starting Express on Port ${config.port_html}...`);
      callback(null);
    });
  }], 
  finish: ['start_express', function(results, callback) {
  }]
});