cmake_minimum_required(VERSION 2.8)

add_executable( asynchttprequest_test main.cpp asynchttprequester.cpp )

find_package( Boost 1.53.0 COMPONENTS chrono system REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )

find_package( CURL REQUIRED )
include_directories( ${CURL_INCLUDE_DIRS} )

target_link_libraries( asynchttprequest_test ${CURL_LIBRARIES} ${Boost_LIBRARIES} )
