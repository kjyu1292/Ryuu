cmake_minimum_required(VERSION 3.28.3)
project(Ryuu C ASM_NASM)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(INC_DIR ${PROJECT_ROOT}/include)
set(SRC_DIR ${PROJECT_ROOT}/src)
set(BIN_DIR ${PROJECT_ROOT}/bin)
set(TEST_UNIT_DIR ${PROJECT_ROOT}/test/unit/prng)
set(TEST_BLEND_DIR ${PROJECT_ROOT}/test/blend/prng)
set(DEBUG_DIR ${PROJECT_ROOT}/debug)

file(MAKE_DIRECTORY ${PROJECT_ROOT}/build)

set(CMAKE_C_COMPILER gcc)
set(C_FLAGS -Wall -Wextra -g) 
set(ASM_FLAGS -g -f elf64)
set(VALGRIND_FLAGS -s --leak-check=full --show-leak-kinds=all --track-origins=yes)

add_custom_target(mt19937_obj
        COMMAND         nasm ${ASM_FLAGS} 
                        -I${INC_DIR} ${SRC_DIR}/_mt19937.s
                        -o ${BIN_DIR}/_mt19937.o
        COMMENT         "Assembling _mt19937.s"
)

add_custom_target(valgrind_all)
add_custom_target(valgrind_testUnit)
add_custom_target(valgrind_testBlend)
add_dependencies(valgrind_all valgrind_testUnit valgrind_testBlend)

add_custom_target(debug_all)

function(create_testUnit_targets test_file)

        get_filename_component(test_name ${test_file} NAME_WE)

        add_custom_target(${test_name}_obj
                COMMAND         ${CMAKE_C_COMPILER} 
				${C_FLAGS}
                                -I${INC_DIR} -c  
                                -o ${BIN_DIR}/${test_name}.o 
                                ${TEST_UNIT_DIR}/${test_name}.c
                COMMENT         "Compiling ${test_name}.c"
                DEPENDS         mt19937_obj
        )
    
        add_custom_target(${test_name}
                COMMAND         ${CMAKE_C_COMPILER}
                                ${C_FLAGS}
                                -o ${BIN_DIR}/${test_name}
                                ${BIN_DIR}/${test_name}.o
                                ${BIN_DIR}/_mt19937.o
                COMMENT         "Linking ${test_name}"
                DEPENDS         ${test_name}_obj
        )

	if(NOT ${test_name} MATCHES "__512$")
        	add_custom_target(valgrind_${test_name}
        	        COMMAND         valgrind
        	                        ${VALGRIND_FLAGS}
        	                        ${BIN_DIR}/${test_name}
        	                        2> ${TEST_UNIT_DIR}/out/${test_name}.txt
        	        COMMENT         "Memory checking ${test_name}.c"
        	        DEPENDS         ${test_name}
        	)
		add_dependencies(valgrind_testUnit valgrind_${test_name})
	endif()

        add_custom_target(debug_${test_name}
                COMMAND         gdb --batch
                                --command=${DEBUG_DIR}/in/${test_name}.gdb
                                ${BIN_DIR}/${test_name}
                                > ${DEBUG_DIR}/out/${test_name}.txt
                                || true
                COMMAND         ex -s '+g/^[^$$]/d' '+g/^$$/d' '+wq'
                                ${DEBUG_DIR}/out/${test_name}.txt
                COMMENT         "Debugging ${test_name}.c"
                DEPENDS         ${test_name}
        )
        add_dependencies(debug_all debug_${test_name})

endfunction()

function(create_testBlend_targets test_file)

        get_filename_component(test_name ${test_file} NAME_WE)

        add_custom_target(${test_name}_obj
                COMMAND         ${CMAKE_C_COMPILER}
				${C_FLAGS}
                                -I${INC_DIR} -c
                                -o ${BIN_DIR}/${test_name}.o
                                ${TEST_BLEND_DIR}/${test_name}.c
                COMMENT         "Compiling ${test_name}.c"
                DEPENDS         mt19937_obj
        )

        add_custom_target(${test_name}
                COMMAND         ${CMAKE_C_COMPILER}
                                ${C_FLAGS}
                                -o ${BIN_DIR}/${test_name}
                                ${BIN_DIR}/${test_name}.o
                                ${BIN_DIR}/_mt19937.o
                COMMENT         "Linking ${test_name}"
                DEPENDS         ${test_name}_obj
        )

	if(NOT ${test_name} MATCHES ".*512$")
        	add_custom_target(valgrind_${test_name}
        	        COMMAND         valgrind
        	                        ${VALGRIND_FLAGS}
        	                        ${BIN_DIR}/${test_name} 1 1 1024 1024
        	                        2> ${TEST_BLEND_DIR}/out/${test_name}.txt
			COMMAND		ex -s '+$a' '+' +'' '+.' '+wq'
					${TEST_BLEND_DIR}/out/${test_name}.txt
        	        COMMENT         "Memory checking ${test_name}.c"
        	        DEPENDS         ${test_name}
        	)
        	add_dependencies(valgrind_testBlend valgrind_${test_name})
	endif()

	if(${test_name} MATCHES ".*runtime.*")
		add_custom_target(${test_name}_runtime
		        COMMAND         ${BIN_DIR}/${test_name} 10 10 1024 1024
					&>> ${TEST_BLEND_DIR}/out/${test_name}.txt
		        COMMENT         "Testing runtime target for ${test_name}"
		        DEPENDS         ${test_name}
			WORKING_DIRECTORY "${TEST_BLEND_DIR}"
		)
		add_dependencies(valgrind_testBlend ${test_name}_runtime)
	endif()

endfunction()

file(GLOB TEST_UNIT_FILES "${TEST_UNIT_DIR}/*.c")
file(GLOB TEST_BLEND_FILES "${TEST_BLEND_DIR}/*.c")

foreach(test_file ${TEST_UNIT_FILES})
        file(MAKE_DIRECTORY ${DEBUG_DIR}/out)
        file(MAKE_DIRECTORY ${TEST_UNIT_DIR}/out)
        create_testUnit_targets(${test_file})
endforeach()

foreach(test_file ${TEST_BLEND_FILES})
        file(MAKE_DIRECTORY ${TEST_BLEND_DIR}/out)
        create_testBlend_targets(${test_file})
endforeach()





