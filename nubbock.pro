QT += gui gui-private core-private waylandcompositor waylandcompositor-private

LIBS += -L ../../lib

HEADERS += \
    accelerometer.h \
    compositor.h \
    inputdevice.h \
    window.h

SOURCES += main.cpp \
    accelerometer.cpp \
    compositor.cpp \
    inputdevice.cpp \
    window.cpp
