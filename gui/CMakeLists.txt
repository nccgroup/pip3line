####
# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS " Configuring pip3line" )
project( pip3line )
cmake_minimum_required( VERSION 2.8.9 )
set ( CMAKE_BUILD_TYPE Release )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
        find_package(Qt5Network)
else(WITH_QT5)
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg QtNetwork QtMain )
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

include_directories (
	../libtransform)

link_directories (
	${PROJECT_SOURCE_DIR}/../lib
	)

set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

set ( EXTERN_LIBS transform)

if (UNIX)
	add_definitions ( -Wall )
endif (UNIX)

set ( pip3line_HDRS
	confgui/transformguimgmt.h
	mainwindow.h
	transformwidget.h
	aboutdialog.h
	messagedialog.h
	analysedialog.h
	massprocessingdialog.h
	regexphelpdialog.h
	customdialogdockwidget.h
	loggerwidget.h
	bytetableview.h
	byteitemmodel.h
	infodialog.h
	screeniodevice.h
	settingsdialog.h
	downloadmanager.h
	pluginconfwidget.h
	guihelper.h
	transformsgui.h
	maintabs.h
	floatingdialog.h
	quickviewdialog.h
	quickviewitem.h
	quickviewitemconfig.h
	rendertextview.h
	textwidget.h
	newbytedialog.h
	hexwidget.h
	debugdialog.h
	../tools/processor.h
	../tools/pipeserver.h
	../tools/streamprocessor.h
	../tools/tcpserver.h
	../tools/binaryprocessor.h
	../tools/textprocessor.h
	../tools/serverabstract.h
	../version.h
	)

set ( pip3line_SRCS
	main.cpp
	win.rc
	mainwindow.cpp
	transformwidget.cpp
	messagedialog.cpp
	aboutdialog.cpp
	analysedialog.cpp
	massprocessingdialog.cpp
	regexphelpdialog.cpp
	customdialogdockwidget.cpp
	loggerwidget.cpp
	bytetableview.cpp
	byteitemmodel.cpp
	infodialog.cpp
	screeniodevice.cpp
	settingsdialog.cpp
	downloadmanager.cpp
	pluginconfwidget.cpp
	guihelper.cpp
	transformsgui.cpp
	maintabs.cpp
	floatingdialog.cpp
	quickviewdialog.cpp
	quickviewitem.cpp
	quickviewitemconfig.cpp
	rendertextview.cpp
	textwidget.cpp
	newbytedialog.cpp
	hexwidget.cpp
	debugdialog.cpp
	../tools/processor.cpp
	../tools/binaryprocessor.cpp
	../tools/textprocessor.cpp
	../tools/pipeserver.cpp
	../tools/streamprocessor.cpp
	../tools/tcpserver.cpp
	../tools/serverabstract.cpp
	)

set ( pip3line_UIS
	mainwindow.ui
	transformwidget.ui
	aboutdialog.ui
	messagedialog.ui
	regexphelpdialog.ui
	analysedialog.ui
	customdialogdockwidget.ui
	loggerwidget.ui
	massprocessingdialog.ui
	insertdialog.ui
	infodialog.ui
	settingsdialog.ui
	pluginconfwidget.ui
	transformsgui.ui
	tabname.ui
	floatingdialog.ui
	quickviewdialog.ui
	quickviewitem.ui
	quickviewitemconfig.ui
	textwidget.ui
	newbytedialog.ui
	hexwidget.ui
	debugdialog.ui
	)

set ( pip3line_RSCS
	gui_res.qrc
	)

if(WITH_QT5)
        qt5_wrap_ui(UIS ${pip3line_UIS})
	qt5_add_resources(RSCS ${pip3line_RSCS})
else(WITH_QT5)
        QT4_WRAP_UI(UIS ${pip3line_UIS})
	QT4_ADD_RESOURCES(RSCS ${pip3line_RSCS})
endif(WITH_QT5)

add_executable ( pip3line WIN32 ${pip3line_SRCS} ${UIS} ${RSCS} )

if(WITH_QT5)
        qt5_use_modules(pip3line Widgets Svg Network )
	if(WIN32)
		target_link_libraries ( pip3line Qt5::WinMain )
	endif(WIN32)
else(WITH_QT5)
       	target_link_libraries ( pip3line  ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( pip3line ${EXTERN_LIBS} )

if(UNIX)
	install (TARGETS pip3line DESTINATION bin)
	install (FILES pip3line.desktop DESTINATION /usr/share/applications)
	install (FILES icons/pip3line.png DESTINATION /usr/share/icons/hicolor/128x128/apps)
endif(UNIX)

