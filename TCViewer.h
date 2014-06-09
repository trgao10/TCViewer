//== INCLUDES =================================================================
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include "TCViewerT.h"

//== CLASS DEFINITION =========================================================
using namespace OpenMesh;  
using namespace OpenMesh::Attributes;

struct MyTraits : public OpenMesh::DefaultTraits
{
    HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

//== CLASS DEFINITION =========================================================
class TCViewer : public TCViewerT<MyMesh>
{
    Q_OBJECT

public:
    /// default constructor
    TCViewer(QWidget* parent=0) : TCViewerT<MyMesh>(parent)
    {}
    OpenMesh::IO::Options& options() { return _options; }
    const OpenMesh::IO::Options& options() const { return _options; }
    void setOptions(const OpenMesh::IO::Options& opts) { _options = opts; }
    
    void open_mesh_gui(QString fname)
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
    void open_texture_gui(QString fname)
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

public slots:
    void query_open_mesh_file() {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open mesh file"),
                                                        tr(""),
                                                        tr("OFF Files (*.off);;"
                                                           "OBJ Files (*.obj);;"
                                                           "STL Files (*.stl);;"
                                                           "All Files (*)"));
        if (!fileName.isEmpty())
            open_mesh_gui(fileName);
    }
    
    
    void query_open_texture_file() {
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

private:
    OpenMesh::IO::Options _options;
};
