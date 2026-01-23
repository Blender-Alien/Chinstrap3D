# Chinstrap3D

A cross-platform custom Game Engine focused on stylized 3D rendering and 
realistic audio simulation with good pre-calculation tooling.

## Tooling Dependencies
- CMake
- Clang (LLVM)
- Ninja

## Library Dependencies 
- GLFW 3.4 ( https://glfw.org )
- spdlog 1.17.0 ( https://github.com/gabime/spdlog )

## How to build

### Download and place all library dependencies source-code

```
vendor/glfw-3.4
vendor/spdlog-1.17.0
```

### Run the following commands **in project root directory**
Generate CMake environment:
```
cmake -G "Ninja Multi-Config" -B bin
```
Build an executable using environment:
```
cmake --build bin --config Debug
```

## How to run

```
./bin/app/Debug/app
```
