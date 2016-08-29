import path from 'path';
import fs from 'fs';
import less from 'less';

import config from './config';

const path_lessFiles = path.resolve(config.path_root, 'src/less');
const less_options = {
	paths: [path_lessFiles],
	doctype: 'html',
	filename: 'public.less',
	compress: true
};

if (global.g_files == null) global.g_files = {};

function render(target) {
  less.render(fs.readFileSync(path.resolve(path_lessFiles, `${target}.less`), 'utf8'), less_options, function (e, output) {
    g_files[`/${target}.min.css`] = output.css;
  });
}

const targets = [
   'admin',
   'judge',
   'public',
   'user'
];

for (var target in targets)
  render(targets[target]);