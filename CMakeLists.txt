cmake_minimum_required(VERSION 3.0.0)
project(ksfeeder VERSION 0.1.0)
message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")

set(PROJECT_PATH ${CMAKE_SOURCE_DIR})
set(TARGET_NAME ${CMAKE_PROJECT_NAME})

file(GLOB ALL_KSFILE "./kitsprout/*.c" )
file(GLOB ALL_INCLUDE "kitsprout")

link_libraries(m)
add_executable(${TARGET_NAME}
    main.c
    online.c
    offline.c
    ksfeeder.c

    ${ALL_KSFILE}
)

add_definitions(
    -DDEFAULE_LOGFILE="LOG_SAMSUNG_SMG973F_20210722_234221.csv"
    -DLOGOUT_RELATE_PATH="result/"
    -DLOGOUT_FILE_TAG="_result"
)

include_directories(
    ${ALL_INCLUDE}
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

add_custom_target(run
    COMMAND ${TARGET_NAME}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_PATH}/build"
)
