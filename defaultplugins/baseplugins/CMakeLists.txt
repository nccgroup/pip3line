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

message ( STATUS " Configuring the base plugin " )
project( baseplugins )
cmake_minimum_required( VERSION 2.8.9 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DBASEPLUGINS_LIBRARY)


set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
        find_package(Qt5Network)
        find_package(Qt5XmlPatterns)
else(WITH_QT5)
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg QtNetwork QtXmlPatterns)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)


include_directories (
	../../libtransform/)

link_directories (
	${PROJECT_SOURCE_DIR}/../../lib
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

set ( baseplugins_HDRS
	baseplugins_global.h
	md4.h
	md5.h
	sha1.h
	baseplugins.h
	substitution.h
	numbertochar.h
	xor.h
	urlencode.h
	split.h
	rotx.h
	reverse.h
	regularexp.h
	randomcase.h
	postgresconcat.h
	padding.h
	oracleconcat.h
	mysqlconcatv2.h
	mysqlconcat.h
	mssqlconcat.h
	javascriptconcat.h
	html.h
	hexencode.h
	cut.h
	charencoding.h
	binary.h
	base64.h
	base32.h
	basex.h
	iptranslateipv4.h
	networkmaskipv4.h
	networkmaskipv6.h
	ciscosecret7.h
	xmlquery.h
	fixprotocol.h
	hieroglyphy.h
	zlib.h
	timestamp.h
	microsofttimestamp.h
	bytesinteger.h
	ntlmssp.h
	confgui/xorwidget.h
	confgui/urlencodewidget.h
	confgui/splitwidget.h
	confgui/rotxwidget.h
	confgui/reversewidget.h
	confgui/regularexpwidget.h
	confgui/randomcasewidget.h
	confgui/paddingwidget.h
	confgui/htmlwidget.h
	confgui/hexencodewidget.h
	confgui/cutwidget.h
	confgui/charencodingwidget.h
	confgui/binarywidget.h
	confgui/base64widget.h
	confgui/base32widget.h
	confgui/xmlquerywidget.h
	confgui/iptranslateipv4widget.h
	confgui/substitutionwidget.h
	confgui/hieroglyphywidget.h
	confgui/zlibwidget.h
	confgui/timestampwidget.h
	confgui/microsofttimestampwidget.h
	confgui/byteintegerwidget.h
	confgui/basexwidget.h
	confgui/ntlmsspwidget.h
	)

set ( baseplugins_SRCS
	md5.cpp
	md4.cpp
	sha1.cpp
	baseplugins.cpp
	substitution.cpp
	numbertochar.cpp
	xor.cpp
	urlencode.cpp
	split.cpp
	rotx.cpp
	reverse.cpp
	regularexp.cpp
	randomcase.cpp
	postgresconcat.cpp
	padding.cpp
	oracleconcat.cpp
	mysqlconcatv2.cpp
	mysqlconcat.cpp
	mssqlconcat.cpp
	javascriptconcat.cpp
	html.cpp
	hexencode.cpp
	cut.cpp
	charencoding.cpp
	binary.cpp
	base64.cpp
	base32.cpp
	basex.cpp
	iptranslateipv4.cpp
	networkmaskipv4.cpp
	networkmaskipv6.cpp
	ciscosecret7.cpp
	xmlquery.cpp
	fixprotocol.cpp
	hieroglyphy.cpp
	zlib.cpp
	timestamp.cpp
	microsofttimestamp.cpp
	bytesinteger.cpp
	ntlmssp.cpp
	confgui/xorwidget.cpp
	confgui/urlencodewidget.cpp
	confgui/splitwidget.cpp
	confgui/rotxwidget.cpp
	confgui/reversewidget.cpp
	confgui/regularexpwidget.cpp
	confgui/randomcasewidget.cpp
	confgui/paddingwidget.cpp
	confgui/htmlwidget.cpp
	confgui/hexencodewidget.cpp
	confgui/cutwidget.cpp
	confgui/charencodingwidget.cpp
	confgui/binarywidget.cpp
	confgui/base64widget.cpp
	confgui/base32widget.cpp
	confgui/xmlquerywidget.cpp
	confgui/iptranslateipv4widget.cpp
	confgui/substitutionwidget.cpp
	confgui/hieroglyphywidget.cpp
	confgui/zlibwidget.cpp
	confgui/timestampwidget.cpp
	confgui/microsofttimestampwidget.cpp
	confgui/byteintegerwidget.cpp
	confgui/basexwidget.cpp
	confgui/ntlmsspwidget.cpp
	)

set ( baseplugins_UIS
	confgui/xorwidget.ui
	confgui/urlencodewidget.ui
	confgui/splitwidget.ui
	confgui/rotxwidget.ui
	confgui/reversewidget.ui
	confgui/regularexpwidget.ui
	confgui/randomcasewidget.ui
	confgui/paddingwidget.ui
	confgui/htmlwidget.ui
	confgui/hexencodewidget.ui
	confgui/cutwidget.ui
	confgui/charencodingwidget.ui
	confgui/binarywidget.ui
	confgui/base64widget.ui
	confgui/base32widget.ui
	confgui/xmlquerywidget.ui
	confgui/iptranslateipv4widget.ui
	confgui/substitutionwidget.ui
	confgui/hieroglyphywidget.ui
	confgui/zlibwidget.ui
	confgui/timestampwidget.ui
	confgui/microsofttimestampwidget.ui
	confgui/byteintegerwidget.ui
	confgui/basexwidget.ui
	confgui/ntlmsspwidget.ui
	)

set ( baseplugins_RSCS
	baseplugins_res.qrc
	)

if(WITH_QT5)
        qt5_wrap_ui(UIS ${baseplugins_UIS})
	QT5_ADD_RESOURCES(RSCS ${baseplugins_RSCS})
else(WITH_QT5)
        QT4_WRAP_UI(UIS ${baseplugins_UIS})
	QT4_ADD_RESOURCES(RSCS ${baseplugins_RSCS})
endif(WITH_QT5)

set ( EXTERN_LIB transform )

add_library ( baseplugins MODULE ${baseplugins_SRCS} ${UIS} ${RSCS} )
if(WITH_QT5)
        qt5_use_modules( baseplugins Widgets Svg Network XmlPatterns )
else(WITH_QT5)
	target_link_libraries ( baseplugins ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( baseplugins ${EXTERN_LIB} )

IF(UNIX)
	install (TARGETS baseplugins DESTINATION share/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)

