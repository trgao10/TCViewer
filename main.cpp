//== INCLUDES =================================================================
#include <iostream>
#include <fstream>
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>

#include "MainWindow.h"
#include "TCViewer.h"

//== MAIN FUNCTION ============================================================
int main(int argc, char** argv)
{
    // OpenGL check
    QApplication::setColorSpec(QApplication::CustomColor);
    QApplication application(argc,argv);
    
    if ( !QGLFormat::hasOpenGL() ) {
        QString msg = "System has no OpenGL support!";
        QMessageBox::critical( 0, QString("OpenGL"), msg + QString(argv[1]) );
        return -1;
    }

    OpenMesh::IO::Options opt;

    /// enable most options for now
    opt += OpenMesh::IO::Options::VertexColor;
    opt += OpenMesh::IO::Options::VertexNormal;
    opt += OpenMesh::IO::Options::VertexTexCoord;
    opt += OpenMesh::IO::Options::FaceColor;
    opt += OpenMesh::IO::Options::FaceNormal;
    opt += OpenMesh::IO::Options::FaceTexCoord;
  
    MainWindow mainWin;
    TCViewer viewer(&mainWin);
    viewer.setOptions(opt);
    mainWin.setCentralWidget(&viewer);
    viewer.setWindowTitle("TCViewer");
    mainWin.createActions(&viewer);
    mainWin.createMenus();
    mainWin.show();
    
    /// load scene if specified on the command line
    if (optind < argc)
    {
        viewer.open_mesh_gui(argv[optind]);
    }

    if ( ++optind < argc )
    {
        viewer.open_texture_gui(argv[optind]);
    }

    return application.exec();
}

