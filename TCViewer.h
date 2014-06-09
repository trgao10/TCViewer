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
    
    void open_mesh_gui(QString fname);
    void open_texture_gui(QString fname);

public slots:
    void query_open_mesh_file();
    void query_open_texture_file();

private:
    OpenMesh::IO::Options _options;

};
