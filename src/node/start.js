import fs from 'fs';
import path from 'path';
import { spawn, exec } from 'child_process';
import isRunning from 'is-running';

var startBeachJudge = null;

// Launch Rethink
(() => {
  const path_rethinkdb_pid = path.resolve(__dirname, '../../rethinkdb.pid');
  function startRethinkDB() {
    const path_rethinkdb = process.platform === 'win32' ? '../../external/rethinkdb/rethinkdb' : '../../external/rethinkdb/build/release/rethinkdb';
    const out = fs.openSync(path.resolve(__dirname, '../../rethinkdb_out.log'), 'a');
    const err = fs.openSync(path.resolve(__dirname, '../../rethinkdb_err.log'), 'a');
    const child = spawn(
      path.resolve(__dirname, path_rethinkdb),
      ['--http-port', 8081],
      {
        detached: true,
        stdio: [
          'ignore',
          out,
          err
        ],
      }
    );

    fs.writeFile(
      path_rethinkdb_pid,
      child.pid,
      (err) => {
        if (err) {
          throw err;
        }
      }
    );
    child.unref();
    console.log(`Started Rethinkdb (${child.pid})`);
    startBeachJudge();
  }

  fs.exists(path_rethinkdb_pid, exists => {
    if (exists) {
      fs.readFile(path_rethinkdb_pid, (err, pid) => {
        if (err) {
          throw err;
        }
        if (isRunning(pid)) {
          console.log(`RethinkDB is already running (${pid})...`);
        } else {
          startRethinkDB();
        }
      });
    }
    else {
      startRethinkDB();
    }
  });
})();


// Launch BeachJudge
(() => {
  startBeachJudge = function() {
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
    console.log(`Started BeachJudge`);
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
}
})();
