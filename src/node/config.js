import fs from 'fs';
import path from 'path';
import merge from 'lodash/merge';

const path_root = path.resolve(__dirname, '../../../');
const path_config = path.resolve(path_root, 'config.json');

let config;
try {
  config = require(path_config);
}
catch (e) {
  config = undefined;
}

// Default config
const defaultConfig = {
  port_html: 8080,
  rethinkdb: {
    host: 'localhost',
    port: 28015,
    authkey: '',
    db: 'beachjudge'
  }
};

export default merge(
  defaultConfig,
  config
);

fs.writeFileSync(path_config, JSON.stringify(config, null, '\t'), 'utf-8');
// var config;
// try {
//   fs.accessSync(path_configFile, fs.constants.R_OK);
//   config = JSON.parse(fs.readFileSync(path_configFile, 'utf8'));
// } catch (e) {
//   config = defaultConfig;
//   fs.writeFileSync(path_configFile, JSON.stringify(config, null, '\t'), 'utf-8');
// }
//
// // Update missing config defaults
// var changed = false;
// for (var p in defaultConfig)
//   if (config[p] == null) {
//     config[p] = defaultConfig[p];
//     changed = true;
//   }
//
//   // Commit changes
// if (changed) {
// }
//
// defaultConfig = null;
//
// // Exports
// config.path_mods = path_mods;
// config.path_root = path_root;
// module.exports = config;
// global.g_config = config;
