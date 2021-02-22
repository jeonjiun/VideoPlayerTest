QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    videosnapshot.cpp

HEADERS += \
    mainwindow.h \
    player.h \
    videosnapshot.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#unix|win32: LIBS += -L$$PWD/QtAV/lib/ -lQtAV1 -lQtAVWidgets1
unix|win32: LIBS += -L$$PWD/vlc-3.0.10/sdk/lib/ -llibvlc -llibvlccore

#INCLUDEPATH += $$PWD/ffmpeg/include
INCLUDEPATH += $$PWD/QtAv/include
INCLUDEPATH += $$PWD/QtAv/include/QtAV
INCLUDEPATH += $$PWD/QtAv/include/QtAVWidgets
INCLUDEPATH += $$PWD/vlc-3.0.10/sdk/include
#DEPENDPATH += $$PWD/ffmpeg/include/libavcodec
#DEPENDPATH += $$PWD/ffmpeg/include/libavcodec
