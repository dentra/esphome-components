#!/bin/bash

set -e

if [ ! -L esphome ]; then
    ln -sv /usr/local/lib/python3.7/dist-packages/esphome esphome
fi
