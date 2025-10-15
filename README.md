
本工具用于辅助BepInEx注入hybridclr的热更函数，原理是把热更函数转换成AOT函数。
This tool is used to assist BepInEx in injecting the hotfix function of hybridclr. The principle is to convert the hot update function into an AOT function.

# 构建

## xmake

本工具使用xmake作为构建工具，不熟悉的可以前往[官网](https://xmake.io/)了解下载

## GameAssembly.dll.lib

编译本库需要对应unity版本的`GameAssembly.dll.lib`放到xmake目录下，用于链接一些实现

## 编译

编译需要用到hybridclr的il2cpp目录

``` bat
xmake f --il2cpp_path=your\path\to\HybridCLRData\LocalIl2CppData-WindowsEditor\il2cpp
xmake build
```

# Build

## xmake

This tool uses xmake as the build tool. If you're unfamiliar with it, please visit the official website for downloading.

## GameAssembly.dll.lib

To compile this library, you need to place the Unity-specific `GameAssembly.dll.lib` file in the xmake directory. This is used to link against some implementations.

## Compile

Compiling requires the HybridCLR il2cpp directory.

``` bat
xmake f --il2cpp_path=your\path\to\HybridCLRData\LocalIl2CppData-WindowsEditor\il2cpp
xmake build
```