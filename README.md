# ![puffer](asset/puffer.png) Protocol Puffers
[![Codacy](https://api.codacy.com/project/badge/Grade/31e7d1d7bcbe43959aaec3b86093b843)](https://app.codacy.com/gh/PragmaTwice/protopuf?utm_source=github.com&utm_medium=referral&utm_content=PragmaTwice/protopuf&utm_campaign=Badge_Grade)
[![Github Actions](https://github.com/PragmaTwice/protopuf/workflows/BuildAndTest/badge.svg)](https://github.com/PragmaTwice/protopuf/actions)
[![Vcpkg Port](https://img.shields.io/badge/vcpkg-protopuf-blue)](https://github.com/microsoft/vcpkg/blob/master/ports/protopuf/vcpkg.json)
[![Codecov](https://codecov.io/gh/PragmaTwice/protopuf/branch/master/graph/badge.svg?token=4EPLZ6Z4J5)](https://codecov.io/gh/PragmaTwice/protopuf)

*A little, highly templated, and protobuf-compatible serialization/deserialization library written in C++20*

:closed_book: [Documentation](https://protopuf.surge.sh)

## Requirements

- a compiler and a standard library implementation with C++20 support 
    - GCC 11 or above, or
    - Clang 12 or above, or
    - MSVC 14.29 (Visual Studio 2019 Version 16.9) or above
- CMake 3
- GoogleTest (optional, for unit tests)
- vcpkg (optional, `vcpkg install protopuf` to install)

## Features

- Data structures are described using type in C++ instead of DSLs like the Protocol Buffer Language (`.proto`)
- Fully compatible with encoding of the Protocol Buffers, capable of mutual serialization/deserialization
- Extensive compile-time operations aimed to improving run-time performance

## An Example
For the following data structure described using `.proto`:
```proto
message Student {
    uint32 id = 1;
    string name = 3;
}

message Class {
    string name = 8;
    repeated Student students = 3;
}
```
We can use *protopuf* to describe it as C++ types:
```c++
using namespace pp;

using Student = message<
    uint32_field<"id", 1>, 
    string_field<"name", 3>
>;

using Class = message<
    string_field<"name", 8>, 
    message_field<"students", 3, Student, repeated>
>;
```
Subsequently, both serialization and deserialization become so easy to do:
```c++
// serialization
Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
Class myClass {"class 101", {tom, jerry}};
myClass["students"_f].push_back(twice);

array<byte, 64> buffer{};
auto bufferEnd = message_coder<Class>::encode(myClass, buffer);
assert(begin_diff(bufferEnd, buffer) == 45);

// deserialization
auto [yourClass, bufferEnd2] = message_coder<Class>::decode(buffer);
assert(yourClass["name"_f] == "class 101");
assert(yourClass["students"_f][2]["name"_f] == "twice");
assert(yourClass["students"_f][2]["id"_f] == 123);
assert(yourClass["students"_f][1] == (Student{123456, "jerry"}));
assert(yourClass == myClass);
assert(begin_diff(bufferEnd2, bufferEnd) == 0);
```
More examples can be found in our test cases ([test/message.cpp](https://github.com/PragmaTwice/protopuf/blob/master/test/message.cpp)).

## Supported Field Types
Category|	Supported Types
--------|------------------
Varint  | int32, int64, uint32, uint64, sint32, sint64, bool, enum
64-bit  | fixed64, sfixed64, double
Length-delimited| string, bytes, embedded messages, packed repeated fields
32-bit 	| fixed32, sfixed32, float

## Worklist
- [x] named field via NTTP ([#6](https://github.com/PragmaTwice/protopuf/pull/6))
- [x] dynamic reflection on field names and numbers ([#9](https://github.com/PragmaTwice/protopuf/pull/9))
- [x] compatibility test between protopuf and protobuf ([#10](https://github.com/PragmaTwice/protopuf/pull/10))
- [x] benchmark test between protopuf and protobuf ([#11](https://github.com/PragmaTwice/protopuf/pull/11))

## Known issues
- There is [a known bug](https://developercommunity2.visualstudio.com/t/Wrong-compile-error-in-MSVC:-identifier-/1270794) related to template parameter lists of lambda expressions in Visual Studio 2019 Version 16.8, which can produce a wrong compilation error while compiling protopuf
- Although class type in NTTP ([P0732R2](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r2.pdf)) is implemented in GCC 10, there is a CTAD bug ([PR96331](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96331), exists until GCC 10.2) to reject valid NTTP usage, which prevent protopuf to compile successfully

## Special Thanks
to [JetBrains](https://www.jetbrains.com/?from=protopuf) for its [Open Source License](https://www.jetbrains.com/community/opensource/?from=protopuf) of All Products Pack

![JetBrains Logo](https://www.jetbrains.com/company/brand/img/jetbrains_logo.png)
