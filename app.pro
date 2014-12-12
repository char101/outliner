TEMPLATE = app
TARGET = outliner
CONFIG += c++11
RC_FILE = res/app.rc
RESOURCES = res/app.qrc
QT += widgets sql
QTPLUGIN.imageformats = -
QTPLUGIN.qmltooling = -
QTPLUGIN.bearer = -
QTPLUGIN.printsupport = -

DEFINES *= QT_USE_QSTRINGBUILDER

INCLUDEPATH += . 3rdparty/hoedown/src
HEADERS = 3rdparty/hoedown/src/*.h src/*.h
SOURCES = 3rdparty/hoedown/src/*.c src/*.cpp

win32 {
	appicon.target = res/app.ico
	appicon.commands = convert.exe res/app16.png res/app.ico
	appicon.depends = res/app16.png

	QMAKE_EXTRA_TARGETS += appicon
	PRE_TARGETDEPS += res/app.ico

	DEFINES += _CRT_SECURE_NO_WARNINGS
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
