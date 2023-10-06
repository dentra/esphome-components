#!/bin/bash

set -e

ln -svFfh $(python3 -c 'import esphome as _; print(_.__path__[0])') esphome

if [ $USER == "vscode" ]; then
  find . -type d \( -name .esphome -or -name .pio \) -exec sudo chown -R vscode:vscode {} \;
else
  ln -svFfh $HOME/.platformio
fi
