TEMPLATE = app
TARGET   = TCViewer

HEADERS  = TCViewerT.h TCViewer.h \
    MainWindow.h
SOURCES  = main.cpp \
    TCViewerT.cpp \
    TCViewer.cpp \
    MainWindow.cpp

QT *= xml opengl widgets gui

# CONFIG += qt opengl warn_on thread rtti console embed_manifest_exe
CONFIG += qt opengl warn_on thread rtti console

INCLUDEPATH *= /usr/include /usr/local/include
LIBS *= -L/usr/lib/QGLViewer -lQGLViewer /usr/local/lib/OpenMesh/libOpenMeshCored.so /usr/local/lib/OpenMesh/libOpenMeshToolsd.so


