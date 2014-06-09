#ifndef TCVIEWERT_H
#define TCVIEWERT_H

//== INCLUDES =================================================================
//#include <OpenMesh/Core/Geometry/VectorT.hh>
//#include <QtOpenGL/qgl.h>
//#include <vector>
//#include <map>
#include <string>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#include <QGLViewer/qglviewer.h>

//== FORWARDS =================================================================
class QImage;

//== CLASS DEFINITION =========================================================
template <typename T>
class TCViewerT : public QGLViewer
{
public:
    typedef T                             Mesh;
    typedef OpenMesh::StripifierT<Mesh>   MyStripifier;

public:
    /// default constructor
    TCViewerT(QWidget* _parent=0)
        : QGLViewer(_parent),
          f_strips_(false), 
          tex_id_(0),
          tex_mode_(GL_MODULATE),
          strips_(mesh_),
          use_color_(true),
          show_vnormals_(false),
          show_fnormals_(false)
          {}
    
    ///destructor
    ~TCViewerT() {}

    /// open mesh
    virtual bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt);
  
    /// load texture
    virtual bool open_texture( const char *_filename );
    bool set_texture( QImage& _texsrc );
 
    void enable_strips();
    void disable_strips();  

    Mesh& mesh() { return mesh_; }
    const Mesh& mesh() const { return mesh_; }
    
    /* QAction* findAction(const char *name) */
    /* { */
    /*     QString namestr = QString(name); */
    /*     ActionMap::iterator e = names_to_actions.end(); */
        
    /*     ActionMap::iterator found = names_to_actions.find(namestr); */
    /*     if (found != e) { */
    /*         return found->second; */
    /*     } */
    /*     return 0; */
    /* } */

    /* void addAction(QAction* act, const char* name) */
    /* { */
    /*     names_to_actions[name] = act; */
    /*     QGLWidget::addAction(act); */
    /* } */
    
    /* void removeAction(const char* name) */
    /* { */
    /*     QString namestr = QString(name); */
    /*     ActionMap::iterator e = names_to_actions.end(); */
        
    /*     ActionMap::iterator found = names_to_actions.find(namestr); */
    /*     if (found != e) { */
    /*         removeAction(found->second); */
    /*     } */
    /* } */
    
    /* void removeAction(QAction* act) */
    /* { */
    /*     ActionMap::iterator it = names_to_actions.begin(), e = names_to_actions.end(); */
    /*     ActionMap::iterator found = e; */
    /*     for(; it!=e; ++it) { */
    /*         if (it->second == act) { */
    /*             found = it; */
    /*             break; */
    /*         } */
    /*     } */
    /*     if (found != e) { */
    /*         names_to_actions.erase(found); */
    /*     } */
    /*     popup_menu_->removeAction(act); */
    /*     draw_modes_group_->removeAction(act); */
    /*     QGLWidget::removeAction(act); */
    /* } */

    
    /* /// add draw mode to popup menu, and return the QAction created */
    /* QAction *add_draw_mode(const std::string& _s){ */
    /*     ++n_draw_modes_; */
    /*     draw_mode_names_.push_back(_s); */
        
    /*     QActionGroup *grp = draw_modes_group_; */
    /*     QAction* act = new QAction(tr(_s.c_str()), this); */
    /*     act->setCheckable(true); */
    /*     act->setData(n_draw_modes_); */
        
    /*     grp->addAction(act); */
    /*     popup_menu_->addAction(act); */
    /*     addAction(act, _s.c_str()); */
        
    /*     return act; */
    /* } */

    /* /// delete draw mode from popup menu */
    /* void del_draw_mode(const std::string& _s) */
    /* { */
    /*     QString cmp = _s.c_str(); */
    /*     QList<QAction*> actions_ = popup_menu_->actions(); */
    /*     QList<QAction*>::iterator it=actions_.begin(), e=actions_.end(); */
    /*     for(; it!=e; ++it) { */
    /*         if ((*it)->text() == cmp) { break; } */
    /*     } */
  
    /*     popup_menu_->removeAction(*it); */
    /* } */

    /* const std::string& current_draw_mode() const  */
    /* { return draw_mode_ ? draw_mode_names_[draw_mode_-1] : nomode_; } */

    
protected :
    void setDefaultMaterial();
    void setDefaultLight();
    
    virtual void draw();
    virtual void init();
    virtual QString helpString() const;

protected: // Strip support
    void compute_strips(void)
    {
        if (f_strips_)
        {
            strips_.clear();
            strips_.stripify();
        }
    }    

protected:   
    bool                   f_strips_; // enable/disable strip usage
    GLuint                 tex_id_;
    GLint                  tex_mode_;
    OpenMesh::IO::Options  opt_; // mesh file contained texcoords?
  
    Mesh                   mesh_;
    MyStripifier           strips_;
    bool                   use_color_;
    bool                   show_vnormals_;
    bool                   show_fnormals_;
    float                  normal_scale_;
    OpenMesh::FPropHandleT< typename Mesh::Point > fp_normal_base_;

/* private: */
/*     // popup menu for draw mode selection */
/*     QMenu*               popup_menu_; */
/*     QActionGroup*        draw_modes_group_; */
/*     typedef std::map<QString,QAction*> ActionMap; */
/*     ActionMap            names_to_actions; */
/*     unsigned int              draw_mode_; */
/*     unsigned int              n_draw_modes_; */
/*     std::vector<std::string>  draw_mode_names_; */
/*     static std::string        nomode_; */
};

#ifndef TCVIEWERT_CPP
#include "TCViewerT.cpp"
#endif
//=============================================================================
#endif // TCVIEWERT_H defined
//=============================================================================
