#include "TCViewer.h"

void TCViewer::Smooth()
{
    set_draw_mode("Smooth");
    updateGL();
}

void TCViewer::Flat()
{
    set_draw_mode("Flat");
    updateGL();
}

void TCViewer::Wireframe()
{
    set_draw_mode("Wireframe");
    updateGL();
}

void TCViewer::Points()
{
    set_draw_mode("Points");
    updateGL();
}

void TCViewer::HiddenLine()
{
    set_draw_mode("Hidden-Line");
    updateGL();
    std::cout << "Hidden-Line" << std::endl;
}

void TCViewer::about()
{
    help();
}

void TCViewer::aboutQt()
{
}

void TCViewer::open_mesh_gui(QString fname)
{
    OpenMesh::Utils::Timer t;
    t.start();
    if ( fname.isEmpty() || !open_mesh(fname.toLocal8Bit(), _options) )
    {
        QString msg = "Cannot read mesh from file:\n '";
        msg += fname;
        msg += "'";
        QMessageBox::critical( NULL, windowTitle(), msg);
    }
    t.stop();
    std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
}

void TCViewer::open_texture_gui(QString fname)
{
    if ( fname.isEmpty() || !open_texture( fname.toLocal8Bit() ) )
    {
        QString msg = "Cannot load texture image from file:\n '";
        msg += fname;
        msg += "'\n\nPossible reasons:\n";
        msg += "- Mesh file didn't provide texture coordinates\n";
        msg += "- Texture file does not exist\n";
        msg += "- Texture file is not accessible.\n";
        QMessageBox::warning( NULL, windowTitle(), msg );
    }
}

void TCViewer::query_open_mesh_file() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open mesh file"),
                                                    tr(""),
                                                    tr("OFF Files (*.off);;"
                                                       "PLY Files (*.ply);;"
                                                       "OBJ Files (*.obj);;"
                                                       "STL Files (*.stl);;"
                                                       "All Files (*)"));
    if (!fileName.isEmpty())
        open_mesh_gui(fileName);
}

void TCViewer::query_open_texture_file() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open texture file"),
                                                    tr(""),
                                                    tr("PNG Files (*.png);;"
                                                       "BMP Files (*.bmp);;"
                                                       "GIF Files (*.gif);;"
                                                       "JPEG Files (*.jpg);;"
                                                       "TIFF Files (*.tif);;"
                                                       "All Files (*)"));
    if (!fileName.isEmpty())
        open_texture_gui(fileName);
}

