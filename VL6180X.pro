QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp\
        madgwickahrs.cpp \
        mainwindow.cpp \
        objectgl.cpp \
        socketcan_cpp.cpp

HEADERS  += mainwindow.h \
    madgwickahrs.h \
    objectgl.h \
    socketcan_cpp.h \
    socketcan_cpp_export.h

FORMS += \
    mainwindow.ui


#LIBS        += -LleRepertoireVersLaLibToto -lpcan
#INCLUDEPATH += /usr/local/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
