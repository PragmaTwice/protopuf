#   Copyright 2020-2021 PragmaTwice
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER message.proto)

add_executable(protopuf_compatibility_test main.cpp ${PROTO_SRC})

target_include_directories(protopuf_compatibility_test PRIVATE ${Protobuf_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protopuf_compatibility_test protopuf ${CMAKE_THREAD_LIBS_INIT} ${GTEST_LIBS} ${Protobuf_LIBRARIES})

gtest_discover_tests(protopuf_compatibility_test)
