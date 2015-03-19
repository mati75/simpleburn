cmake_minimum_required(VERSION 2.8)

set (PROJECT_NAME "simpleburn")
project (${PROJECT_NAME})
set (PROJECT_VERSION "1.7.3")

add_subdirectory (po)
add_subdirectory (src)
add_subdirectory (scripts)
install (DIRECTORY doc/ DESTINATION doc/${PROJECT_NAME}-${PROJECT_VERSION})
option (GTK3 "Enable gtk+3.")

add_custom_target (uninstall)
add_custom_command (TARGET uninstall COMMAND xargs rm -f < ${CMAKE_BINARY_DIR}/install_manifest.txt)
add_custom_command (TARGET uninstall COMMAND rmdir ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${PROJECT_VERSION})
add_custom_command (TARGET uninstall COMMAND rmdir ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
