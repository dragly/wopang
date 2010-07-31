# #####################################################################
# Automatically generated by qmake (2.01a) Thu Oct 4 19:01:12 2007
# #####################################################################
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += main.cpp \
    model.cpp \
    ui/window.cpp \
    ui/ui.cpp \
    ui/controller.cpp \
    ui/cbutton.cpp \
    node.cpp \
    ui/hudobject.cpp \
    ui/radar.cpp \
    ui/hudicon.cpp \
    entity.cpp \
    unit.cpp \
    tank.cpp \
    soundbank.cpp \
    soundthread.cpp
SOURCES += glwidget.cpp
SOURCES += mainwindow.cpp
SOURCES += glm.cpp
HEADERS += glwidget.h \
    model.h \
    ui/window.h \
    ui/ui.h \
    ui/controller.h \
    ui/cbutton.h \
    node.h \
    ui/hudobject.h \
    ui/radar.h \
    ui/hudicon.h \
    entity.h \
    unit.h \
    tank.h \
    soundbank.h \
    soundthread.h
HEADERS += mainwindow.h
HEADERS += glm.h
RESOURCES +=
QT += opengl \
    multimedia
CONFIG += mobility
MOBILITY += sensors

unix {
    #QMAKE_POST_LINK = cp -R ../loose_cannon/data .;
}
win32 {
    #QMAKE_POST_LINK = xcopy ..\loose_cannon\data\*.* data /e;
}
