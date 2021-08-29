find_package(Catch2 3 REQUIRED)

add_executable(tests lib/tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_library(ahrs lib/ahrs.cpp)

add_executable(example lib/example.c)
target_link_libraries(example PRIVATE ahrs)

