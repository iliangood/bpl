find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ../include)

add_executable(stackTest stack/stackTest.cpp)
target_link_libraries(stackTest stack ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
add_test(NAME stackTest COMMAND stackTest)

#add_executable(variablesTest variables/typesTest.cpp)
#target_link_libraries(variablesTest variables ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
#add_test(NAME variablesTest COMMAND variablesTest)