#!/bin/bash

cmake -G "Ninja Multi-Config" -B bin

ln -s bin/compile_commands.json .
