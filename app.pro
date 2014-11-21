TEMPLATE = app
TARGET = outliner
INCLUDEPATH += . 3rdparty/hoedown/src
CONFIG += windows c++11
RC_FILE = res/app.rc
RESOURCES = res/app.qrc
DEFINES += _CRT_SECURE_NO_WARNINGS

QT += widgets sql
QTPLUGIN.imageformats = -

HEADERS = 3rdparty/hoedown/src/*.h src/*.h
SOURCES = 3rdparty/hoedown/src/*.c src/*.cpp

CONFIG(release, release|debug) {
	DESTDIR = build$(SUFFIX)/release
	OBJECTS_DIR = build$(SUFFIX)/release/obj
	MOC_DIR = build$(SUFFIX)/release/moc
	RCC_DIR = build$(SUFFIX)/release/qrc
	UI_DIR = build$(SUFFIX)/release/ui
}
CONFIG(debug, release|debug) {
	DESTDIR = build$(SUFFIX)/debug
	OBJECTS_DIR = build$(SUFFIX)/debug/obj
	MOC_DIR = build$(SUFFIX)/debug/moc
	RCC_DIR = build$(SUFFIX)/debug/qrc
	UI_DIR = build$(SUFFIX)/debug/ui
}
