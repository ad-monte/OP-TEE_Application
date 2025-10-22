project (ta_secret C)

set (SRC host/main.c
		host/light_crypto_host.c)

add_executable (${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
			   PRIVATE ta/include
			   PRIVATE host/include
			   PRIVATE include)

target_link_libraries (${PROJECT_NAME} PRIVATE teec)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# second executable for test suite
set (SRC_test interface/test_suite.c)
add_executable(test_suite ${SRC_test})

target_include_directories(test_suite 
			   PRIVATE ta/include
			   PRIVATE include)

target_link_libraries (test_suite PRIVATE teec)
install (TARGETS test_suite DESTINATION ${CMAKE_INSTALL_BINDIR})