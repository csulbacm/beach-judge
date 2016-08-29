var path = require('path');
var fs = require('fs');
var http = require('http');

var path_root = path.resolve(__dirname, '../..');
var path_mods = path.resolve(path_root, 'build/external/nodejs/node_modules')
var path_configFile = path.resolve(path_root, 'build/config.json');

// Default config
var defaultConfig = {
  port_html: 8080,
  rethinkdb: {
    host: 'localhost',
    port: 28015,
    authkey: '',
    db: 'beachjudge'
  }
};

var config;
try {
  fs.accessSync(path_configFile, fs.constants.R_OK);
  config = JSON.parse(fs.readFileSync(path_configFile, 'utf8'));
} catch (e) {
  config = defaultConfig;
  fs.writeFileSync(path_configFile, JSON.stringify(config, null, '\t'), 'utf-8');
}

// Update missing config defaults
var changed = false;
for (var p in defaultConfig)
  if (config[p] == null) {
    config[p] = defaultConfig[p];
    changed = true;
  }

  // Commit changes
if (changed) {
  fs.writeFileSync(path_configFile, JSON.stringify(config, null, '\t'), 'utf-8');
}

defaultConfig = null;

// Exports
config.path_mods = path_mods;
config.path_root = path_root;
module.exports = config;
global.g_config = config;
