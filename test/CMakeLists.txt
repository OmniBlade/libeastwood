include_directories(${LIBEASTWOOD_SOURCE_DIR}/include)

add_executable(testy test.cpp)
IF(LIBEASTWOOD_BUILD_SDL)
add_executable(cpsview cpsview.cpp)
target_link_libraries(cpsview eastwood SDL_eastwood ${SDL_LIBRARY})
ENDIF(LIBEASTWOOD_BUILD_SDL)
target_link_libraries(testy eastwood)