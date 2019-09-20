@echo off

if not exist "..\build" (
    mkdir ..\build
)

pushd ..\build
"c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" ^
 -DLLVM_ENABLE_PROJECTS=clang;clang-tools-extra ^
 -DLLVM_BUILD_EXAMPLES=OFF ^
 -DLLVM_INCLUDE_TESTS=OFF ^
 -DLLVM_BUILD_EXAMPLES=OFF ^
 -DLLVM_INCLUDE_EXAMPLES=OFF ^
 -DLLVM_BUILD_BENCHMARKS=OFF ^
 -DLLVM_INCLUDE_BENCHMARKS=OFF ^
 -A x64 -Thost=x64 ..\llvm
popd
