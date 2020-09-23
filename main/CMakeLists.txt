include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

idf_component_register(SRCS "src/UpdateHelper.cpp"
        INCLUDE_DIRS "include"
        REQUIRES mbedtls)