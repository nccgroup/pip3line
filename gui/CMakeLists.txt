####
# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring pip3line gui --" )
project( pip3line )
set ( CMAKE_BUILD_TYPE Release )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
        find_package(Qt5Network)
	find_package(Qt5Concurrent)
else(WITH_QT5)
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg QtNetwork QtMain )
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

include_directories (
	../libtransform)

if (NOT APPLE)
  link_directories (
    ${PROJECT_SOURCE_DIR}/../lib
  )
endif()

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
	loggerwidget.h
	infodialog.h
	screeniodevice.h
	settingsdialog.h
	downloadmanager.h
	pluginconfwidget.h
	guihelper.h
	maintabs.h
	floatingdialog.h
	quickviewdialog.h
	quickviewitem.h
	quickviewitemconfig.h
	newbytedialog.h
	debugdialog.h
	textinputdialog.h
	comparisondialog.h
	appdialog.h
	../tools/processor.h
	../tools/pipeserver.h
	../tools/streamprocessor.h
	../tools/tcpserver.h
	../tools/binaryprocessor.h
	../tools/textprocessor.h
	../tools/serverabstract.h
	../tools/processingstats.h
	../version.h
	sources/basicsource.h
	sources/bytesourceabstract.h
	sources/currentmemorysource.h
	sources/largerandomaccesssource.h
	sources/memorywidget.h
	sources/filewidget.h
	sources/largefile.h
	sources/blocksources/blockssource.h
	sources/blocksources/udplistener.h
	sources/blocksources/tcpserverlistener.h
	sources/blocksources/tcplistener.h
	sources/searchabstract.h
	sources/intercept/intercepsource.h
	sources/intercept/interceptwidget.h
	sources/intercept/payload.h
	sources/intercept/payloadmodel.h
	sources/intercept/interceptorchestratorabstract.h
	sources/intercept/interceptorproxy.h
	sources/networkconfwidget.h
	sources/intermediatesource.h
	sources/networkclientwidget.h
	sources/blocksources/rawtcplistener.h
	views/hexview.h
	views/textview.h
	views/bytetableview.h
	views/byteitemmodel.h
	views/singleviewabstract.h
	views/foldedview.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/generictab.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/randomaccesstab.h
	shared/hexwidget.h
	shared/offsetgotowidget.h
	shared/searchwidget.h
	shared/readonlybutton.h
	shared/clearallmarkingsbutton.h
	shared/bytesourceguibutton.h
	shared/detachtabbutton.h
	shared/universalreceiverbutton.h
	shared/messagepanelwidget.h
	shared/searchresultswidget.h
	state/basestateabstract.h
	state/stateorchestrator.h
	state/closingstate.h
	state/statedialog.h
	state/statestatuswidget.h
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
	loggerwidget.cpp
	infodialog.cpp
	screeniodevice.cpp
	settingsdialog.cpp
	downloadmanager.cpp
	pluginconfwidget.cpp
	guihelper.cpp
	maintabs.cpp
	floatingdialog.cpp
	quickviewdialog.cpp
	quickviewitem.cpp
	quickviewitemconfig.cpp
	newbytedialog.cpp
	debugdialog.cpp
	textinputdialog.cpp
	comparisondialog.cpp
	appdialog.cpp
	../tools/processor.cpp
	../tools/binaryprocessor.cpp
	../tools/textprocessor.cpp
	../tools/pipeserver.cpp
	../tools/streamprocessor.cpp
	../tools/tcpserver.cpp
	../tools/serverabstract.cpp
	../tools/processingstats.cpp
	sources/basicsource.cpp
	sources/bytesourceabstract.cpp
	sources/currentmemorysource.cpp
	sources/largerandomaccesssource.cpp
	sources/memorywidget.cpp
	sources/filewidget.cpp
	sources/largefile.cpp
	sources/blocksources/blockssource.cpp
	sources/blocksources/udplistener.cpp
	sources/blocksources/tcpserverlistener.cpp
	sources/blocksources/tcplistener.cpp
	sources/searchabstract.cpp
	sources/intercept/intercepsource.cpp
	sources/intercept/interceptwidget.cpp
	sources/intercept/payload.cpp
	sources/intercept/payloadmodel.cpp
	sources/intercept/interceptorchestratorabstract.cpp
	sources/intercept/interceptorproxy.cpp
	sources/networkconfwidget.cpp
	sources/intermediatesource.cpp
	sources/networkclientwidget.cpp
	sources/blocksources/rawtcplistener.cpp
	views/hexview.cpp
	views/bytetableview.cpp
	views/byteitemmodel.cpp
	views/textview.cpp
	views/singleviewabstract.cpp
	views/foldedview.cpp
	tabs/generictab.cpp
	tabs/tababstract.cpp
	tabs/transformsgui.cpp
	tabs/transformsgui.cpp
	tabs/tababstract.cpp
	tabs/randomaccesstab.cpp
	shared/hexwidget.cpp
	shared/offsetgotowidget.cpp
	shared/searchwidget.cpp
	shared/readonlybutton.cpp
	shared/clearallmarkingsbutton.cpp
	shared/bytesourceguibutton.cpp
	shared/detachtabbutton.cpp
	shared/universalreceiverbutton.cpp
	shared/messagepanelwidget.cpp
	shared/searchresultswidget.cpp
	state/basestateabstract.cpp
	state/stateorchestrator.cpp
	state/closingstate.cpp
	state/statedialog.cpp
	state/statestatuswidget.cpp
	)

set ( pip3line_UIS
	mainwindow.ui
	comparisondialog.ui
	transformwidget.ui
	aboutdialog.ui
	messagedialog.ui
	regexphelpdialog.ui
	analysedialog.ui
	loggerwidget.ui
	massprocessingdialog.ui
	insertdialog.ui
	infodialog.ui
	settingsdialog.ui
	pluginconfwidget.ui
	tabs/transformsgui.ui
	tabs/generictab.ui
	tabs/transformsgui.ui
	tabs/randomaccesstab.ui
	floatingdialog.ui
	quickviewdialog.ui
	quickviewitem.ui
	quickviewitemconfig.ui
	newbytedialog.ui
	shared/hexwidget.ui
	shared/messagepanelwidget.ui
	debugdialog.ui
	sources/memorywidget.ui
	sources/filewidget.ui
	comparisondialog.ui
	textinputdialog.ui
	views/hexview.ui
	views/textview.ui
	views/foldedview.ui
	sources/intercept/interceptwidget.ui
	sources/networkconfwidget.ui
	sources/networkclientwidget.ui
	shared/searchresultswidget.ui
	state/statedialog.ui
	state/statestatuswidget.ui
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
        qt5_use_modules(pip3line Widgets Svg Network Concurrent)
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

