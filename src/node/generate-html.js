import path from 'path';
import fs from 'fs';
import pug from 'pug';

import config from './config';

const path_pugStaticFiles = path.resolve(config.path_root, 'src/pug/static');
const path_pugDynamicFiles = path.resolve(config.path_root, 'src/pug/dynamic');

const pug_options = {
	doctype: 'html',
	pretty: true
};

// TODO: Use compile for dynamic and render for static
// TODO: Minify

if (global.g_pages == null) global.g_pages = {};

pug_options.filename = path.resolve(path_pugStaticFiles, 'login.pug');
g_pages['/'] = pug.renderFile(pug_options.filename, pug_options);