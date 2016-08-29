import fs from 'fs';
import isRunning from 'is-running';

import config from './config';

function startServer() {
  fs.writeFile(
    config.path_beachjudge_pid,
    process.pid,
    err => {
      if (err) {
        throw err;
      }
      require('./server');
    }
  );
}

fs.exists(config.path_beachjudge_pid, exists => {
  if (exists) {
    fs.readFile(config.path_beachjudge_pid, (err, pid) => {
      if (err) {
        throw err;
      }
      if (isRunning(pid)) {
        console.log(`BeachJudge is already running (${pid}), exiting...`);
        process.exit(1);
      }
      startServer();
    });
  }
  else {
    startServer();
  }
});
