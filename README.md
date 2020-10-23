# ![puffer](asset/puffer.png) Protocol Puffers
[![Github Actions](https://github.com/PragmaTwice/protopuf/workflows/BuildAndTest/badge.svg)](https://github.com/PragmaTwice/protopuf/actions)

*What is this?*

A little, highly templated, and protobuf-compatible serialization/deserialization library written in C++20

## Requirements

- a compiler and a standard library implementation with C++20 support 
    - only GCC 10 and its libstdc++ now
- GoogleTest (optional)
- CMake 3

## Features

- Data structures are described using type in C++ instead of DSLs like the Protocol Buffer Language (`.proto`)
- Fully compatible with encoding of the Protocol Buffers, capable of mutual serialization/deserialization
- Extensive compile-time operations aimed to improving run-time performance
