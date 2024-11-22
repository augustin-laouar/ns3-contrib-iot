check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/iot-camera-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME iot-camera
    SOURCE_FILES model/iot-camera.cc
                 helper/iot-camera-helper.cc
    HEADER_FILES model/iot-camera.h
                 helper/iot-camera-helper.h
    LIBRARIES_TO_LINK ${libcore}
    TEST_SOURCES test/iot-camera-test-suite.cc
                 ${examples_as_tests_sources}
)
