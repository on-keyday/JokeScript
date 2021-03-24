cmake_minimum_required (VERSION 3.8)


include_directories("D:\\OpenSSL\\build-x64-debug\\include")
link_directories("D:\\OpenSSL\\build-x64-debug\\lib")

add_executable (httpclient "HTTPClient.cpp" "HTTPClient.h" "common/stdcpps.h" "common/structs.cpp" "common/structs.h" "common/io.h" "common/io.cpp" "common/log.h" "common/filereader.h" "common/filereader.cpp"  "common/log.cpp" "common/ctype.h" "common/ctype.cpp" "common/tools.h" "common/tools.cpp" "json_tools.h" "json_tools.cpp" "main.cpp")

target_link_libraries(httpclient wsock32 ws2_32 libcrypto libssl)


