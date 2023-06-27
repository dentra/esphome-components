#!/bin/bash

set -e

function info { echo -e "\e[32m[info] $*\e[39m"; }
function warn { echo -e "\e[33m[warn] $*\e[39m"; }
function error { echo -e "\e[31m[error] $*\e[39m"; exit 1; }

source $(dirname $0)/post-create-env

pio_ini=platformio.ini

build_dir=".esphome"
build_include_dir="$build_dir/include"

sedi() {
  expr=$1
  file=$2
  # Failed on VirtioFS, so workaraund it
  # sed -i "$expr" "$file"
  temp=$(mktemp)
  sed "$expr" "$file" > $temp && mv $temp $file
}

ext_compo_url() {
  d=$1
  a=(${d//@/ })
  url=${a[0]}
  label=${a[1]}
  name="${url%.git}"
  name=(${name//\// })
  name="$build_include_dir/${name[${#name[@]} - 1]}/esphome"
  mkdir -p $(dirname "$name")
  rm -rf "$name"
  if [ "$label" == "" ]; then
    git clone --depth 1 "$url" -c advice.detachedHead=false "$name"
  else
    git clone --depth 1 --branch "$label" "$url" -c advice.detachedHead=false "$name"
  fi
  echo "    -I $(dirname $name)" >> "$pio_ini"
}

ext_compo() {
  d=$1
  info "Configure external component $d..."
  if [[ "$d" =~ https?:\/\/* ]] ; then
    ext_compo_url $d
  elif [[ "$d" == github://* ]] ; then
    ext_compo_url "https://github.com/${d#github://}"
  else
    c="$build_include_dir/$d"
    rm -rf $c
    mkdir -p "$c/esphome"
    ln -sf $(realpath ".esphome/external_components/$d/components") "$c/esphome/components"
    echo "    -I $c" >> "$pio_ini"
  fi
}

# take ownership of mounted folders
files=(.esphome .pio .platformio)
for f in "${files[@]}" ; do
  if [ ! -d "$f" ]; then
    mkdir "$f"
  else
    sudo chown -R vscode:vscode "$f"
  fi
done

files=($pio_ini .clang-format .clang-tidy .editorconfig pylintrc)
for f in "${files[@]}" ; do
  info "Updating $f..."
  curl -Ls "https://github.com/esphome/esphome/raw/dev/$f" -o "$f"
done

c_dir="$build_include_dir/components"
mkdir -p "$c_dir/esphome"
ln -sf $(realpath "components") "$c_dir/esphome/"

# replace "esphome" to virtual components src
sedi "/src_dir/s/esphome/${c_dir//\//\\/}/" "$pio_ini"
# replace ".temp" to ".esphome in "sdkconfig_path"
sedi "/sdkconfig_path/s/.temp/.esphome/" "$pio_ini"


sedi "s/-DESPHOME_LOG_LEVEL/\${prj.build_flags}\n    -DESPHOME_LOG_LEVEL/" "$pio_ini"
sedi "s/esphome\/noise-c@/\${prj.lib_deps}\n    esphome\/noise-c@/" "$pio_ini"

echo "" >> "$pio_ini"
echo "# project specific settings" >> "$pio_ini"
echo "[prj]" >> "$pio_ini"
echo "build_flags =" >> "$pio_ini"
for d in "${defines[@]}" ; do
  echo "    -D$d" >> "$pio_ini"
done
for d in "${includes[@]}" ; do
  echo "    -I $d" >> "$pio_ini"
done
for d in "${external_components[@]}" ; do
  ext_compo $d
done
echo "lib_deps =" >> "$pio_ini"
for d in "${lib_deps[@]}" ; do
  echo "    $d" >> "$pio_ini"
done


sedi "s/include_dir = ./include_dir = .\nlibdeps_dir=.esphome\/libdeps\nbuild_dir=.esphome\/build/" "$pio_ini"

cpp_json=.vscode/c_cpp_properties.json

# export PLATFORMIO_CORE_DIR=".platformio"
export PLATFORMIO_LIB_DIR="/none"
export PLATFORMIO_GLOBALLIB_DIR=""

# if [ "$(find /esphome/.temp/platformio/* -maxdepth 0 -type d 2>/dev/null|wc -l)" -lt "1" ]; then
  info "Updating vscode cpp environment to $pio_env..."
  pio init --ide vscode --silent -e $pio_env
  # sedi "/\\/workspaces\/esphome\/include/d" $cpp_json
  # rm -rf CMakeLists.txt components/CMakeLists.txt
# else
  # echo "Cpp environment already configured. To reconfigure it you could run one the following commands:"
  # echo "  pio init --ide vscode -e esp8266-arduino"
  # echo "  pio init --ide vscode -e esp32-arduino"
  # echo "  pio init --ide vscode -e esp32-idf"
# fi
rmdir .pio

# additionally remove annoying pio ide recomendations
sedi "/platformio.platformio-ide/d" .vscode/extensions.json
