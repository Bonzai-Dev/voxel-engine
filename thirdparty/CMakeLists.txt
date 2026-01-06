# SDL
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# glad
add_subdirectory(glad)

# glm
add_subdirectory(glm)

#simdjson
add_subdirectory(simdjson)

#stb
add_library(stb STATIC stb/stb.c)
target_include_directories(stb PUBLIC "stb")
