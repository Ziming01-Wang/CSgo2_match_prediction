cmake_minimum_required(VERSION 3.16)

project(cs2_predictor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_PREFIX_PATH STREQUAL "" AND EXISTS "D:/Qt/5.14.2/mingw73_64/lib/cmake/Qt5")
  list(APPEND CMAKE_PREFIX_PATH "D:/Qt/5.14.2/mingw73_64")
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)

if(MSVC)
  add_compile_options(/utf-8)
else()
  add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_executable(cs2_predictor WIN32
  resources/resources.qrc
  src/main.cpp
  src/domain/Catalog.cpp
  src/domain/Preview.cpp
  src/app/Session.h
  src/app/Session.cpp
  src/ui/Theme.cpp
  src/ui/Kit.cpp
  src/ui/widgets/WinBar.cpp
  src/ui/MainWindow.cpp
  src/ui/pages/HomePage.cpp
  src/ui/pages/TeamPage.cpp
  src/ui/pages/PredictPage.cpp
  src/ui/pages/MajorPage.cpp
  src/ui/pages/ArchivePage.cpp
)

target_include_directories(cs2_predictor PRIVATE src)

target_link_libraries(cs2_predictor PRIVATE Qt5::Widgets)

if(WIN32 AND TARGET Qt5::WinMain)
  target_link_libraries(cs2_predictor PRIVATE Qt5::WinMain)
endif()
