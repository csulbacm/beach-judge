import fs from 'fs';
import path from 'path';
import merge from 'lodash/merge';

// get config
const path_root = path.resolve(__dirname, '../../../');
const path_config = path.resolve(path_root, 'config.json');

let config;
try {
  config = require(path_config);
}
catch (e) {
  config = undefined;
}

// default config
const defaultConfig = {
  port_html: 8080,
  rethinkdb: {
    host: 'localhost',
    port: 28015,
    authkey: '',
    db: 'beachjudge'
  }
};

// assign config over default config
export default merge(
  defaultConfig,
  config
);

// save merged config
fs.writeFileSync(path_config, JSON.stringify(config, null, '\t'), 'utf-8');
