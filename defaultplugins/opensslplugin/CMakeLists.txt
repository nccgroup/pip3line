####
# pip3line: easy encoder/decoder and more
# Copyright (C) 2012-2013  Gabriel Caudrelier<gabriel.caudrelier@gmail.com>
#
# Pip3line is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Pip3line is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pip3line.  If not, see <http://www.gnu.org/licenses/>.
####

message( STATUS " Configuring the OpenSSL plugin")
project( opensslplugin )

cmake_minimum_required( VERSION 2.8.9 )
cmake_policy(SET CMP0015 NEW)
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DOPENSSLPLUGIN_LIBRARY)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
else(WITH_QT5)
        find_package ( Qt4 REQUIRED QtCore QtGui QtSvg)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

include_directories (
	../../libtransform/
	)

link_directories (
	${PROJECT_SOURCE_DIR}/../../lib
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

find_package(OpenSSL)

if (OPENSSL_FOUND)
	message( STATUS "OpenSSL library found ... OK")
	include_directories(${OPENSSL_INCLUDE_DIR})
	link_directories(${OPENSSL_LIBRARIES})
	if (UNIX)
		set ( EXTERN_LIBS transform crypto )
	endif (UNIX)

	if (WIN32)
		set ( EXTERN_LIBS transform libeay32 )
		link_directories (
			C:\\OpenSSL-Win32\\lib\\
			C:\\OpenSSL-Win64\\lib\\
			)
		include_directories (
			C:\\OpenSSL-Win32\\include\\
			C:\\OpenSSL-Win64\\include\\
			)
	endif (WIN32)


	set ( opensslplugin_HDRS
		opensslplugin_global.h
		opensslplugin.h
		opensslhashes.h
		)

	set ( opensslplugin_SRCS
		opensslplugin.cpp
		opensslhashes.cpp
		)

	if(WITH_QT5)
        	qt5_wrap_ui(UIS ${opensslplugin_UIS})
	else(WITH_QT5)
        	QT4_WRAP_UI(UIS ${opensslplugin_UIS})
	endif(WITH_QT5)
	
	add_library ( opensslplugin MODULE ${opensslplugin_SRCS} ${UIS} )
	if(WITH_QT5)
        	qt5_use_modules(opensslplugin Widgets Svg)
	else(WITH_QT5)
		target_link_libraries ( opensslplugin  ${QT_LIBRARIES} )
	endif(WITH_QT5)

	target_link_libraries ( opensslplugin  ${EXTERN_LIBS} )

	IF(UNIX)
		install (TARGETS opensslplugin DESTINATION share/pip3line/
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
	ENDIF(UNIX)

else (OPENSSL_FOUND)
	message( WARNING "OpenSSL library not found !! this plugin will not be compiled." )
endif (OPENSSL_FOUND)


