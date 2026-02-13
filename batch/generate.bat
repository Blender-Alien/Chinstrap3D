cmake -G "Ninja Multi-Config" -B bin

mklink /H .\compile_commands.json bin\compile_commands.json
