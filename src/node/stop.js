import path from 'path';
import fs from 'fs';
import { exec } from 'child_process';

const path_root = path.resolve(__dirname, '../../../');

function kill(path_pid) {
  fs.exists(path_pid, (exists) => {
    if (exists) {
      fs.readFile(path_pid, (err, pid) => {
        if (err) {
          throw err;
        }
        const cb = (error, stdout, stderr) => {
          if (error) {
            return console.error(`Failed to kill ${path_pid} (${pid})`, error);
          }
          console.log(`Killed ${path_pid} (${pid})`);
        };
        if (process.platform === 'win32') {
          exec(`taskkill /f /pid ${pid}`, cb);
        }
        else {
          exec(`kill -9 ${pid}`, cb);
        }
      });
    }
  });
}

kill(path.resolve(path_root, 'build/beachjudge.pid'));
kill(path.resolve(path_root, 'build/rethinkdb.pid'));
