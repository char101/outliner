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
}
CONFIG(release, release|debug) {
	DEFINES += QT_NO_DEBUG_OUTPUT
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
