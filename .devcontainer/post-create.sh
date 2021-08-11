#!/bin/bash

set -e

pio_ini=platformio.ini
if [ ! -f $cpp_json ]; then
  curl -Ls https://github.com/esphome/esphome/raw/dev/platformio.ini -o platformio.ini
fi

cpp_json=.vscode/c_cpp_properties.json
if [ ! -f $cpp_json ]; then
    pio init --ide vscode --silent
    sed -i "/\\/workspaces\/esphome\/include/d" $cpp_json
else
    echo "Cpp environment already configured. To reconfigure it you could run one the following commands:"
    echo "  pio init --ide vscode -e livingroom8266"
    echo "  pio init --ide vscode -e livingroom32"
fi

