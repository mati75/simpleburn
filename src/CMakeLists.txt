include (FindPkgConfig)

if (GTK2)
pkg_check_modules (LIBS REQUIRED libcdio libcddb dvdread gtk+-2.0)
else (GTK2)
pkg_check_modules (LIBS REQUIRED libcdio libcddb dvdread gtk+-3.0)
endif (GTK2)

set (LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)
set (UI_FILE ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/${PROJECT_NAME}.ui)
configure_file (config.h.cmake config.h)
include_directories (${${PROJECT_NAME}_BINARY_DIR}/src)

#~ set (CMAKE_C_FLAGS "-std=c99")
add_library(mediainfos mediainfos.c)
add_library(progress progress.c)
add_library(callbacks callbacks.c)
add_executable (${PROJECT_NAME} ${PROJECT_NAME}.c)
include_directories (${LIBS_INCLUDE_DIRS}) 
target_link_libraries (simpleburn callbacks mediainfos progress ${LIBS_LDFLAGS} -lm)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (FILES ${PROJECT_NAME}.ui DESTINATION share/${PROJECT_NAME})
install (FILES ${PROJECT_NAME}.desktop DESTINATION share/applications)
install (FILES ${PROJECT_NAME}.png DESTINATION share/icons/hicolor/32x32/apps)

install (PROGRAMS simpleburn.sh DESTINATION bin)
