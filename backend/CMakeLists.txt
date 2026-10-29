cmake_minimum_required(VERSION 3.25)

project(SmartTravelBackend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------- Dependencies ----------
include(FetchContent)

set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  crow
  URL https://github.com/CrowCpp/Crow/archive/refs/tags/v1.3.4.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM
)
FetchContent_MakeAvailable(crow)

find_package(Threads REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)

# vcpkg and source builds ship a CMake config; distro packages (apt) only ship pkg-config.
find_package(libpqxx CONFIG QUIET)
if(TARGET libpqxx::pqxx)
  set(PQXX_TARGET libpqxx::pqxx)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx>=7.0)
  set(PQXX_TARGET PkgConfig::PQXX)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ARGON2 REQUIRED IMPORTED_TARGET libargon2)

# ---------- Application ----------
add_executable(smart_travel_backend
  main.cpp
  src/AppConfig.cpp
  src/Server.cpp
  database/Database.cpp
  controllers/HealthController.cpp
  routes/HealthRoutes.cpp
  routes/FallbackRoutes.cpp
  routes/ApiRoutes.cpp
  services/AuthService.cpp
  services/ContentService.cpp
  services/DashboardService.cpp
  services/ExpenseService.cpp
  services/ItineraryPlanner.cpp
  services/TripService.cpp
  utils/Cookies.cpp
  utils/Env.cpp
  utils/Http.cpp
  utils/JsonBody.cpp
  utils/Password.cpp
  utils/Random.cpp
)

target_include_directories(smart_travel_backend PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(smart_travel_backend PRIVATE
  Crow::Crow
  nlohmann_json::nlohmann_json
  ${PQXX_TARGET}
  PkgConfig::ARGON2
  Threads::Threads
)

if(MSVC)
  target_compile_options(smart_travel_backend PRIVATE /W4 /permissive-)
else()
  target_compile_options(smart_travel_backend PRIVATE -Wall -Wextra -Wpedantic)
endif()