SOURCEPATH += . \
    ../../../libsyncfwclient 
    
INCLUDEPATH += ../ \
			../../msyncd 
			

QT += xml \
    dbus
CONFIG += qdbus \
    link_pkgconfig \

PKGCONFIG += dbus-1
LIBS += 

SOURCES +=  SyncClientInterfaceTest.cpp 

HEADERS +=  SyncClientInterfaceTest.h

OTHER_FILES += 


