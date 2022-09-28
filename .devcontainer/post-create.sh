#!/bin/bash

set -e

sedi() {
  expr=$1
  file=$2
  # Failed on VirtioFS, so workaraund it
  # sed -i "$expr" "$file"
  temp=$(mktemp)
  sed "$expr" "$file" > $temp && mv $temp $file
}

# take ownership of mounted folders
files=(.esphome .pio)
for f in "${files[@]}" ; do
  if [ ! -d "$f" ]; then
    mkdir "$f"
  else
    sudo chown -R vscode:vscode "$f"
  fi
done

pio_ini=platformio.ini

files=($pio_ini .clang-format .clang-tidy .editorconfig .pylintrc)
for f in "${files[@]}" ; do
  if [ ! -f "$f" ] || [ "$f" = "$pio_ini" ]; then
    curl -Ls "https://github.com/esphome/esphome/raw/dev/$f" -o ".esphome/$f"
    ln -svf ".esphome/$f" "$f"
  fi
done

# replace "esphome" to "." in src_dir. esphome linked at post-start.sh
sedi "/src_dir/s/esphome/\./" ".esphome/$pio_ini"
# replace ".temp" to ".esphome in "sdkconfig_path"
sedi "/sdkconfig_path/s/.temp/.esphome/" ".esphome/$pio_ini"

# add project specific defines
# defines=()
# for d in "${defines[@]}" ; do
#   defines_r="$defines_r\n    -D$d"
# done
# sedi "s/ESPHOME_LOG_LEVEL_VERY_VERBOSE/ESPHOME_LOG_LEVEL_VERY_VERBOSE$defines_r/" ".esphome/$pio_ini"

cpp_json=.vscode/c_cpp_properties.json
#if [ ! -f $cpp_json ]; then
if [ "$(find /esphome/.temp/platformio/* -maxdepth 0 -type d 2>/dev/null|wc -l)" -lt "1" ]; then
    pio init --ide vscode --silent -e esp32-arduino
    sedi "/\\/workspaces\/esphome\/include/d" $cpp_json
    rm -rf CMakeLists.txt components/CMakeLists.txt
else
    echo "Cpp environment already configured. To reconfigure it you could run one the following commands:"
    echo "  pio init --ide vscode -e esp8266-arduino"
    echo "  pio init --ide vscode -e esp32-arduino"
    echo "  pio init --ide vscode -e esp32-idf"
fi

# additionally remove annoying pio ide recomendations
sedi "/platformio.platformio-ide/d" .vscode/extensions.json

