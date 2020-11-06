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

using Student = message<uint32_field<1>, string_field<3>>;
using Class = message<string_field<8>, message_field<3, Student, repeated>>;
```
Subsequently, both serialization and deserialization become so easy to do:
```c++
// serialization
Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
Class myClass {"class 101", {tom, jerry}};
myClass[3_f].push_back(twice);

array<byte, 64> buffer;
auto bufferEnd = message_coder<Class>::encode(myClass, buffer);
assert(begin_diff(bufferEnd, buffer) == 45);

// deserialization
auto [yourClass, bufferEnd2] = message_coder<Class>::decode(buffer);
assert(yourClass[8_f] == "class 101");
assert(yourClass[3_f][2][3_f] == "twice");
assert(yourClass[3_f][2][1_f] == 123);
assert(yourClass[3_f][1], Student{123456, "jerry"});
assert(yourClass == myClass);
assert(begin_diff(bufferEnd2, bufferEnd) == 0);
```

## Supported Field Types
Category|	Supported Types
--------|------------------
Varint  | int32, int64, uint32, uint64, sint32, sint64
64-bit  | fixed64, sfixed64, double
Length-delimited| string, bytes, embedded messages, packed repeated fields
32-bit 	| fixed32, sfixed32, float

## Worklist
- [ ] named field via NTTP (issue [#1](https://github.com/PragmaTwice/protopuf/issues/1))
    - shelved due to a CTAD bug in GCC 10 & 11 (exists until now, [PR96331](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96331))
- [ ] better deduction for initializer list
