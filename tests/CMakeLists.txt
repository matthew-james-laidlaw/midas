add_executable(testmidas ObservableTests.cpp WatcherTests.cpp)
target_link_libraries(testmidas observable gtest_main)
