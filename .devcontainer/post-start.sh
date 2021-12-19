#!/bin/bash

set -e

if [ ! -L esphome ]; then
    ln -sv $(python3 -c "import esphome as _; print(_.__path__[0])") esphome
fi
