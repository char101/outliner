TEMPLATE = app
TARGET = outliner
CONFIG += c++11
RC_FILE = res/app.rc
RESOURCES = res/app.qrc
QT += widgets sql qml
QTPLUGIN.imageformats = -

INCLUDEPATH += . 3rdparty/hoedown/src
HEADERS = 3rdparty/hoedown/src/*.h src/*.h
SOURCES = 3rdparty/hoedown/src/*.c src/*.cpp

win32 {
	DEFINES += _CRT_SECURE_NO_WARNINGS

	# Q_COMPILER_INITIALIZER_LISTS is not set for VisualStudio 2013
	# https://bugreports.qt-project.org/browse/QTBUG-39142
    # works for 2013 Community
    # DEFINES += Q_COMPILER_INITIALIZER_LISTS
}
CONFIG(release, release|debug) {
	# QT_NO_DEBUG disable Q_ASSERT
	# QT_NO_DEBUG_OUTPUT disable qDebug
	DEFINES += QT_NO_DEBUG QT_NO_DEBUG_OUTPUT
	QMAKE_CXXFLAGS += /GL
	QMAKE_LFLAGS += /LTCG
	DESTDIR = build/release
	OBJECTS_DIR = build/release/obj
	MOC_DIR = build/release/moc
	RCC_DIR = build/release/qrc
	UI_DIR = build/release/ui
}
CONFIG(debug, release|debug) {
	TARGET = $$TARGET-debug
	DESTDIR = build/debug
	OBJECTS_DIR = build/debug/obj
	MOC_DIR = build/debug/moc
	RCC_DIR = build/debug/qrc
	UI_DIR = build/debug/ui
}
