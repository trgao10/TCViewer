//== INCLUDES =================================================================
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>

#include "TCViewerT.h"
#include "MainWindow.h"

//== CLASS DEFINITION =========================================================
using namespace OpenMesh;  
using namespace OpenMesh::Attributes;

struct TCTraits : public OpenMesh::DefaultTraits
{
    VertexTraits
    {
        typedef OpenMesh::Vec3f Color;
    private:
        float  valence;
        Color valence_color;

    public:
        void set_valence (const float _val) { valence=_val; }
        float get_valence () { return valence; }

        void set_valence_color (Color _val_color) { valence_color=_val_color; }
        Color get_valence_color () { return valence_color; }
    };
};

typedef OpenMesh::TriMesh_ArrayKernelT<TCTraits>  TCMesh;

//== CLASS DEFINITION =========================================================
class TCViewer : public TCViewerT<TCMesh>
{
    Q_OBJECT

public:
    /// default constructor
    TCViewer(QWidget* parent=0) : TCViewerT<TCMesh>(parent)
    {
    }
    OpenMesh::IO::Options& options() { return _options; }
    const OpenMesh::IO::Options& options() const { return _options; }
    void setOptions(const OpenMesh::IO::Options& opts) {  VertexAttributes( OpenMesh::Attributes::Normal |
                                                                            OpenMesh::Attributes::Color );_options = opts; }

    /// open mesh
    virtual bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt);

    /// load texture
    virtual bool open_texture( const char *_filename );
    bool set_texture( QImage& _texsrc );

    void open_mesh_gui(QString fname);
    void open_texture_gui(QString fname);

    /// interpolate [0,1] into RGB valus
    Vec3f interp_color(float _val);
    Vec3f interp_color(float _val, float range_min, float range_max);

    qglviewer::Vec OMVec3f_to_QGLVec(OpenMesh::Vec3f OMVec3f)
    { return qglviewer::Vec(OMVec3f.values_[0], OMVec3f.values_[1], OMVec3f.values_[2]); }

public slots:
    void query_open_mesh_file();
    void query_open_texture_file();

protected:
    virtual void draw();
    virtual void init();

private:
    OpenMesh::IO::Options _options;

private slots:
    void Smooth();
    void Flat();
    void Wireframe();
    void Points();
    void HiddenLine();
    void Valence();

    void about();
    void aboutQt();
};
