# SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig REQUIRED)
pkg_check_modules(CMOCKA IMPORTED_TARGET cmocka)

find_program(BASH bash HINTS /bin /usr/bin REQUIRED)
find_program(DIFF diff REQUIRED)
find_program(TEE tee REQUIRED)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.25")
    try_compile(HAVE_LD_WRAP
        SOURCES
            "${CMAKE_SOURCE_DIR}/cmake/try_compile/ldwrap.c"
        LINK_OPTIONS
            "-Wl,--wrap=exit"
    )
else()
    try_compile(HAVE_LD_WRAP "${CMAKE_BINARY_DIR}/ldwrap"
        SOURCES
            "${CMAKE_SOURCE_DIR}/cmake/try_compile/ldwrap.c"
        LINK_OPTIONS
            "-Wl,--wrap=exit"
    )
endif()

function(blogc_executable_test _libname _src)
    if (NOT CMOCKA_FOUND)
        message(STATUS "Skipping test (cmocka not found): ${_libname}_check_${_src}")
        return()
    endif()

    cmake_parse_arguments(_blogc_executable_test
        ""
        ""
        "WRAP"
        ${ARGN}
    )

    if(DEFINED _blogc_executable_test_WRAP)
        if(NOT HAVE_LD_WRAP)
            message(STATUS "Skipping test (no ld wrap): ${_libname}_check_${_src}")
            return()
        endif()
    endif()

    add_executable(${_libname}_check_${_src}
        check_${_src}.c
    )

    if(DEFINED _blogc_executable_test_WRAP)
        list(JOIN _blogc_executable_test_WRAP ",--wrap=" _wrap)
        set_target_properties(${_libname}_check_${_src}
            PROPERTIES LINK_FLAGS
            "-Wl,--wrap=${_wrap}"
        )
    endif()

    target_link_libraries(${_libname}_check_${_src} PRIVATE
        PkgConfig::CMOCKA
        lib${_libname}
    )

    add_test(
        NAME ${_libname}_check_${_src}
        COMMAND
            ${CMAKE_SOURCE_DIR}/cmake/scripts/test.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${_libname}_check_${_src}
    )
endfunction()

function(blogc_script_test _libname _src)
    configure_file(
        check_${_src}.sh.in
        ${_libname}_check_${_src}.sh
        @ONLY
    )
    add_test(
        NAME ${_libname}_check_${_src}.sh
        COMMAND
            ${CMAKE_SOURCE_DIR}/cmake/scripts/test.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${_libname}_check_${_src}.sh
    )
endfunction()

add_subdirectory(common)
add_subdirectory(blogc)

if(BUILD_BLOGC_MAKE OR BUILD_BLOGC_MAKE_EMBEDDED)
    add_subdirectory(blogc-make)
endif()

if(BUILD_BLOGC_RUNSERVER)
    add_subdirectory(blogc-runserver)
endif()

if(BUILD_BLOGC_GIT_RECEIVER)
    add_subdirectory(blogc-git-receiver)
endif()
