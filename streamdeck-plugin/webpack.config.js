/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

const path = require('path');

const config = {
  entry: {
    pi_bundle: './src/pi_main.ts',
    plugin_bundle: './src/plugin_main.ts',
  },
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: '[name].js'
  },
  resolve: {
    extensions: [".ts", ".tsx", ".js"],
  },
  module: {
    rules: [
      {
        test: /\.m?js$/,
        exclude: /(node_modules|bower_components)/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['@babel/preset-env']
          }
        }
      },
      {
        test: /\.tsx?$/,
        loader: "ts-loader",
      }
    ]
  }
};

module.exports = (env, argv) => {
  if (argv.mode == 'development') {
    config.devtool = 'eval-source-map';
  }
  return config;
};
