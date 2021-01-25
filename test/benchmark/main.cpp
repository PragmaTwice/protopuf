//   Copyright 2020-2021 PragmaTwice
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <protopuf/message.h>
#include <benchmark/benchmark.h>
#include <message.pb.h>

using namespace pp;
using namespace std;

using Student = message<uint32_field<"id", 1>, string_field<"name", 3>>;
using Class = message<string_field<"name", 8>, message_field<"students", 3, Student, repeated>>;

void BM_protopuf_encode(benchmark::State& state) {
    Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
    Class myClass {"class 101", {tom, jerry, twice}};

    array<byte, 64> buffer{};

    for(auto _ : state) {
        message_coder<Class>::encode(myClass, buffer);
    }
}
BENCHMARK(BM_protopuf_encode);

void BM_protobuf_encode(benchmark::State& state) {
    pb::Class yourClass;
    yourClass.set_name("class 101");
    pb::Student* tom = yourClass.add_students();
    tom->set_id(456);
    tom->set_name("tom");
    pb::Student* jerry = yourClass.add_students();
    jerry->set_id(123456);
    jerry->set_name("jerry");
    pb::Student* twice = yourClass.add_students();
    twice->set_id(123);
    twice->set_name("twice");

    array<byte, 64> buffer{};

    for(auto _ : state) {
        yourClass.SerializeToArray(buffer.data(), buffer.size());
    }
}
BENCHMARK(BM_protobuf_encode);

void BM_protopuf_decode(benchmark::State& state) {
    array<byte, 64> buffer {
        0x42_b, 0x9_b, 0x63_b, 0x6c_b, 0x61_b,
        0x73_b, 0x73_b, 0x20_b, 0x31_b, 0x30_b,
        0x31_b, 0x1a_b, 0x8_b, 0x8_b, 0xc8_b,
        0x3_b, 0x1a_b, 0x3_b, 0x74_b, 0x6f_b,
        0x6d_b, 0x1a_b, 0xb_b, 0x8_b, 0xc0_b,
        0xc4_b, 0x7_b, 0x1a_b, 0x5_b, 0x6a_b,
        0x65_b, 0x72_b, 0x72_b, 0x79_b, 0x1a_b,
        0x9_b, 0x8_b, 0x7b_b, 0x1a_b, 0x5_b,
        0x74_b, 0x77_b, 0x69_b, 0x63_b, 0x65_b};

    for(auto _ : state) {
        auto [myClass, _2] = message_coder<Class>::decode(buffer);
        benchmark::DoNotOptimize(myClass);
    }
}
BENCHMARK(BM_protopuf_decode);

void BM_protobuf_decode(benchmark::State& state) {
    array<byte, 64> buffer {
        0x42_b, 0x9_b, 0x63_b, 0x6c_b, 0x61_b,
        0x73_b, 0x73_b, 0x20_b, 0x31_b, 0x30_b,
        0x31_b, 0x1a_b, 0x8_b, 0x8_b, 0xc8_b,
        0x3_b, 0x1a_b, 0x3_b, 0x74_b, 0x6f_b,
        0x6d_b, 0x1a_b, 0xb_b, 0x8_b, 0xc0_b,
        0xc4_b, 0x7_b, 0x1a_b, 0x5_b, 0x6a_b,
        0x65_b, 0x72_b, 0x72_b, 0x79_b, 0x1a_b,
        0x9_b, 0x8_b, 0x7b_b, 0x1a_b, 0x5_b,
        0x74_b, 0x77_b, 0x69_b, 0x63_b, 0x65_b};

    for(auto _ : state) {
        pb::Class myClass;
        myClass.ParseFromArray(buffer.data(), buffer.size());
        benchmark::DoNotOptimize(myClass);
    }
}
BENCHMARK(BM_protobuf_decode);

BENCHMARK_MAIN();
