cmake_minimum_required(VERSION 3.20)
project(ACES_Standalone_NUOPC LANGUAGES CXX)

# The standalone driver is built as an executable
add_executable(aces_nuopc_driver main.cpp)

# Link against the ACES library and its dependencies
target_link_libraries(aces_nuopc_driver
  PRIVATE
  aces
  ESMF::ESMF
  MPI::MPI_CXX
)

# Installation
install(TARGETS aces_nuopc_driver DESTINATION bin)
