#ifndef TCVIEWERT_H
#define TCVIEWERT_H

//== INCLUDES =================================================================
#include <string>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#include <QGLViewer/qglviewer.h>

//== FORWARDS =================================================================
class QImage;
class QMenu;

//== CLASS DEFINITION =========================================================
template <typename T>
class TCViewerT : public QGLViewer
{
public:
    typedef T                             Mesh;

public:
    /// default constructor
    TCViewerT(QWidget* _parent=0)
        : QGLViewer(_parent),
          tex_id_(0),
          tex_mode_(GL_MODULATE),
          use_color_(true),
          show_vnormals_(false),
          show_fnormals_(false),
          draw_mode_("Smooth")
          {}
    
    ///destructor
    ~TCViewerT() {}

    /// open mesh
    virtual bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt);

    /// load texture
    virtual bool open_texture( const char *_filename );
    bool set_texture( QImage& _texsrc );

    /// set draw_mode_
    void set_draw_mode(const std::string _mode);

    Mesh& mesh() { return mesh_; }
    const Mesh& mesh() const { return mesh_; }

    qglviewer::Vec OMVec3f_to_QGLVec(OpenMesh::Vec3f OMVec3f)
    { return qglviewer::Vec(OMVec3f.values_[0], OMVec3f.values_[1], OMVec3f.values_[2]); }

protected :
    void setDefaultMaterial();
    void setDefaultLight();
    
    virtual void draw();
    virtual void init();
    virtual QString helpString() const;
    virtual void postDraw();

    virtual void keyPressEvent(QKeyEvent *e);

protected:
    GLuint                 tex_id_;
    GLint                  tex_mode_;
    OpenMesh::IO::Options  opt_; // mesh file contained texcoords?
  
    Mesh                   mesh_;
    bool                   use_color_;
    bool                   show_vnormals_;
    bool                   show_fnormals_;
    float                  normal_scale_;
    OpenMesh::FPropHandleT< typename Mesh::Point > fp_normal_base_;

    std::string            draw_mode_;
};

#ifndef TCVIEWERT_CPP
#include "TCViewerT.cpp"
#endif
//=============================================================================
#endif // TCVIEWERT_H defined
//=============================================================================
