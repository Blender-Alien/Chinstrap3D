#!/bin/bash

cmake -G "Ninja Multi-Config" -B bin

ln -s bin/compile_commands.json .

# For windows run this in cmd NOT powershell:
# mklink /H .\compile_commands.json bin\compile_commands.json
