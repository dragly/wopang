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
    ui/cbutton.cpp
SOURCES += glwidget.cpp
SOURCES += mainwindow.cpp
SOURCES += glm.cpp
HEADERS += glwidget.h \
    model.h \
    ui/window.h \
    ui/ui.h \
    ui/controller.h \
    ui/cbutton.h
HEADERS += mainwindow.h
HEADERS += glm.h
RESOURCES += texture.qrc
QT += opengl \
    phonon
