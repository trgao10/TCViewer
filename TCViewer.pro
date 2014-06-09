TEMPLATE = app
TARGET   = TCViewer

HEADERS  = TCViewerT.h TCViewer.h
SOURCES  = main.cpp \
    TCViewerT.cpp

QT *= xml opengl widgets gui

CONFIG += qt opengl warn_on thread rtti console embed_manifest_exe

INCLUDEPATH *= /usr/include /usr/local/include
LIBS *= -L/usr/lib/QGLViewer -lQGLViewer /usr/local/lib/OpenMesh/libOpenMeshCored.so /usr/local/lib/OpenMesh/libOpenMeshToolsd.so


