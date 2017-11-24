QT += gui gui-private core-private waylandcompositor waylandcompositor-private

LIBS += -L ../../lib

HEADERS += \
    compositor.h \
    window.h

SOURCES += main.cpp \
    compositor.cpp \
    window.cpp
