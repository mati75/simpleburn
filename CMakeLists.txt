cmake_minimum_required(VERSION 2.8)

set (PROJECT_NAME "simpleburn")
project (${PROJECT_NAME})
set (PROJECT_VERSION "1.6.5")

set (DETECTION "UDEV" CACHE STRING "Device and media detection method (LIBCDIO or UDEV).")
set (BURNING "CDRTOOLS" CACHE STRING "Burning suite (CDRTOOLS or LIBBURNIA).")
option (DEBUG "Enable debug informations.")
option (ALLOW_DEVICES_SYMLINKS "When using LIBCDIO detection method, allow symbolic links for devices." ON)

add_subdirectory (po)
add_subdirectory (src)
add_subdirectory (scripts)
install (DIRECTORY doc/ DESTINATION doc/${PROJECT_NAME}-${PROJECT_VERSION})

add_custom_target (uninstall)
add_custom_command (TARGET uninstall COMMAND xargs rm -f < ${CMAKE_BINARY_DIR}/install_manifest.txt)
add_custom_command (TARGET uninstall COMMAND rmdir ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${PROJECT_VERSION})
add_custom_command (TARGET uninstall COMMAND rmdir ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
