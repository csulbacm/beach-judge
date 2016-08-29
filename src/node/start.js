import fs from 'fs';
import path from 'path';
import { spawn, exec } from 'child_process';

// Launch Rethink
(() => {
  const path_rethinkdb = process.platform === 'win32' ? '../../external/rethinkdb/rethinkdb' : '../../external/rethinkdb/build/release/rethinkdb';
  const child = spawn(
    path.resolve(__dirname, path_rethinkdb),
    ['--http-port', 8081],
    {
      detached: true,
      stdio: [
        'ignore',
        'ignore',
        // fs.openSync(path.resolve(__dirname, '../../rethinkdb_out.log'), 'a'),
        'ignore',
        // fs.openSync(path.resolve(__dirname, '../../rethinkdb_err.log'), 'a'),
      ],
    }
  );

  fs.writeFile(
    path.resolve(__dirname, '../../rethinkdb.pid'),
    child.pid,
    (err) => {
      if (err) {
        throw err;
      }
    }
  );
  child.unref();
  console.log(`Started Rethinkdb (${child.pid})`);
})();


// Launch BeachJudge
(() => {
  const out = fs.openSync(path.resolve(__dirname, '../../beachjudge_out.log'), 'a');
  const err = fs.openSync(path.resolve(__dirname, '../../beachjudge_err.log'), 'a');
  const server = path.resolve(__dirname, '../../generated/node/index.js');
  const path_node = process.platform === 'win32' ? '../../external/nodejs/node' : '../../external/nodejs/bin/node';
  const node = path.resolve(__dirname, path_node);

  let child;
  if (process.platform === 'win32') {
    child = spawn(
      'cmd.exe',
      ['/c', node, server],
      {
        detached: true,
        stdio: ['ignore', out, err]
      }
    );
  }
  else {
    child = spawn(
      node,
      [server],
      {
        detached: true,
        stdio: [
          'ignore',
          out,
          err,
        ],
      }
    );
    console.log(`Started BeachJudge (${child.pid})`);
  }
  child.unref();
})();
