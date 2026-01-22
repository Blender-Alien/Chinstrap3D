# Chinstrap3D

A stylized rendering focussed engine

## Dependencies
- CMake
- Clang (LLVM)
- Ninja

## Dependencies with source-code needed 
- GLFW 3.4
- spdlog 1.17.0

## How to build

### Make sure you are using clang for C/C++
env. variable for **C**
```
export CC=/usr/bin/clang
```
env. variable for **C++**
```
export CXX=/usr/bin/clang++
```

### Download Dependencies
Make sure that all build Dependencies are met

Follow spdlog static library install guide on GitHub

Place uncompressed folder "glfw-3.4" folder into
```
vendor/
```

### Run the following commands **in project root directory**
Generate CMake environment:
```
bash shell/generate.sh
```
Build an exexcutable using environment:
```
bash shell/build-debug.sh
```

## How to run

### Run the following commands in **in project root directory**
Run the executable:
```
bash shell/run-debug.sh
```

## Notes
- Shell scripts only work on linux, eventually they should replaced by a CMake workflow/preset
