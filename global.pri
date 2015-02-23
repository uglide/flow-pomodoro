# qtHaveModule() doesn't work with qml only modules
!exists($$[QT_INSTALL_QML]/QtQuick/Controls/TabView.qml) {
    error("QtQuickControls module was not found")
}

win32:!mingw {
	# Some Qt 5.4 madness going on here, link explicitly
	LIBS += Shell32.lib
}

INCLUDEPATH += src

CONFIG += c++11

# Features:
# DEFINES += DEVELOPER_MODE
# DEFINES += FLOW_DEBUG_TIMESTAMPS

contains(DEFINES, DEVELOPER_MODE) {
    CONFIG += debug
    *-g++*|*clang* {
        QMAKE_CXXFLAGS += -Werror -Wall -Wextra
    }
}
