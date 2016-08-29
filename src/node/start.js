import fs from 'fs';
import path from 'path';
import { spawn } from 'child_process';

// Launch Rethink
(() => {
  const rethinkdbPath = process.platform === 'win32' ? '../../external/rethinkdb/rethinkdb' : '../../external/rethinkdb/build/release/rethinkdb';
  const child = spawn(
    path.resolve(__dirname, rethinkdbPath),
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


// Launch beachJudge
(() => {
  const out = fs.openSync(path.resolve(__dirname, '../../beachjudge_out.log'), 'a');
  const err = fs.openSync(path.resolve(__dirname, '../../beachjudge_err.log'), 'a');
  const index = path.resolve(__dirname, '../../generated/node/server.js');
  const nodePath = process.platform === 'win32' ? '../../external/nodejs/node' : '../../external/nodejs/bin/node';
  const exe = path.resolve(__dirname, nodePath);

  let child;
  if (process.platform === 'win32') {
    child = spawn(
      'cmd.exe',
      ['/c', exe, index],
      {
        detached: true,
        stdio: ['ignore', out, err]
      }
    );
  }
  else {
    child = spawn(
      exe,
      [index],
      {
        detached: true,
        stdio: [
          'ignore',
          out,
          err,
        ],
      }
    );
  }
  if (process.platform !== 'win32') {
    fs.writeFile(
      path.resolve(__dirname, '../../beachjudge.pid'),
      child.pid,
      (err) => {
        if (err) {
          throw err;
        }
      }
    );
    console.log(`Started BeachJudge (${child.pid})`);
  } else {
    console.log(`Started BeachJudge`);
  }
  // TODO: Capture errors from script execution and display them to this script's stdout stream
  child.unref();
})();
