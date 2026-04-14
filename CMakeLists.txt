cmake_minimum_required(VERSION 3.14)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5) # <--- ADD THIS LINE to fix the JSON library error
project(FeatherForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- ADD THIS LINE TO FIX THE DLL ISSUE ---
set(BUILD_SHARED_LIBS OFF)

# --- 1. Fetch SFML (Graphics & Audio) ---
include(FetchContent)
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        2.6.1
)
FetchContent_MakeAvailable(sfml)

# --- 2. Fetch Box2D (Physics Engine) ---
FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG        v2.4.1
)
FetchContent_MakeAvailable(box2d)

# --- 3. Fetch JSON (Level Loading) ---
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# --- 4. Define Sources ---
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_executable(FeatherForge ${SOURCES} ${HEADERS})

# --- 5. Link Libraries ---
target_include_directories(FeatherForge PRIVATE include)

# ---> FIX: ADDED sfml-audio RIGHT HERE! <---
target_link_libraries(FeatherForge PRIVATE sfml-graphics sfml-window sfml-system sfml-audio box2d nlohmann_json::nlohmann_json)

# Copy assets to build directory (Optional but recommended)
add_custom_command(TARGET FeatherForge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:FeatherForge>/assets)