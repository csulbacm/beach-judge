import fs from 'fs';
import path from 'path';
import cp from 'child_process';

// Launch Rethink
(() => {
  const child = cp.spawn(
    path.resolve(__dirname, '../build/external/rethinkdb/rethinkdb'),
    ['--http-port', 8081],
    {
      detached: true,
      stdio: [
        'ignore',
        'ignore',
        // fs.openSync(path.resolve(__dirname, '../build/rethinkdb_out.log'), 'a'),
        'ignore',
        // fs.openSync(path.resolve(__dirname, '../build/rethinkdb_err.log'), 'a'),
      ],
    }
  );

  fs.writeFile(
    path.resolve(__dirname, '../build/rethinkdb.pid'),
    child.pid,
    (err) => {
      if (err) {
        throw err;
      }
    }
  );
  child.unref();
})();


// Launch beachJudge
(() => {
  const out = fs.openSync(path.resolve(__dirname, '../build/beachjudge_out.log'), 'a');
  const err = fs.openSync(path.resolve(__dirname, '../build/beachjudge_err.log'), 'a');
  const index = path.resolve(__dirname, '../build/node/generated/server.js');
  const exe = path.resolve(__dirname, '../build/external/nodejs/node');

  let child;
  if (process.platform === 'windows') {
    child = cp.spawn(
      'cmd.exe',
      ['/c', 'start', '/min', exe, index],
      {
        detached: true,
        stdio: ['ignore', out, err]
      }
    );
  }
  else {
    child = cp.spawn(
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
  // TODO: Capture errors from script execution and display them to this script's stdout stream
  child.unref();
})();
