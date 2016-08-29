import url from 'url';
import qs from 'querystring';
import path from 'path';
import express from 'express';
import bodyParser from 'body-parser';
import fs from 'fs';

import config from './config';

if (process.platform === 'win32')
	fs.writeFile(path.resolve(config.path_root, 'build/beachjudge.pid'), process.pid, (err) => { if (err) throw err; });

const app = express();

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: true
}));

app.listen(config.port_html, () => {
  console.log(`Starting Express on Port ${config.port_html}...`);
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
