#!/bin/bash

set -e

pio_ini=platformio.ini

files=($pio_ini .clang-format .clang-tidy .editorconfig)

for f in "${files[@]}" ; do
  if [ ! -f "$f" ]; then
    curl -Ls "https://github.com/esphome/esphome/raw/dev/$f" -o "$f"
  fi
done


# replace "esphome" to "." in src_dir. esphome linked at post-start.sh
sed -i -e "/src_dir/s/esphome/\./" $pio_ini
# replace ".temp" to ".esphome in "sdkconfig_path"
sed -i -e "/sdkconfig_path/s/.temp/.esphome/" $pio_ini

files=(.esphome .pio)
for f in "${files[@]}" ; do
  if [ ! -d "$f" ]; then
    mkdir "$f"
  else
    sudo chown -R vscode:vscode "$f"
  fi
done

cpp_json=.vscode/c_cpp_properties.json
if [ ! -f $cpp_json ]; then
    pio init --ide vscode --silent
    sed -i "/\\/workspaces\/esphome\/include/d" $cpp_json
    rm CMakeLists.txt components/CMakeLists.txt
else
    echo "Cpp environment already configured. To reconfigure it you could run one the following commands:"
    echo "  pio init --ide vscode -e esp8266"
    echo "  pio init --ide vscode -e esp32"
    echo "  pio init --ide vscode -e esp32-idf"
fi

# additionally remove annoying pio ide recomendations
sed -i "/platformio.platformio-ide/d" .vscode/extensions.json

sudo chown vscode lib/.esphome
sudo chown vscode lib/miot/.esphome
