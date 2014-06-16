#define TCVIEWERT_CPP

//== INCLUDES =================================================================
#include <iostream>
#include <fstream>
// --------------------
#include <QMenu>
#include <QMouseEvent>
#include <QMap>
#include <QCursor>
#include <QImage>
#include <QFileInfo>
#include <QKeyEvent>
// --------------------
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#include "TCViewerT.h"
#include <math.h>

using namespace qglviewer;
using namespace std;
using namespace OpenMesh;
using namespace Qt;

//== IMPLEMENTATION ========================================================== 
template <typename M>
bool 
TCViewerT<M>::open_mesh(const char* _filename, IO::Options _opt)
{
  /// load mesh
  /// calculate normals
  /// set scene center and radius
  
  mesh_.request_face_normals();
  mesh_.request_face_colors();
  mesh_.request_vertex_normals();
  mesh_.request_vertex_colors();
  mesh_.request_vertex_texcoords2D();
  
  std::cout << "Loading from file '" << _filename << "'\n";
  if ( IO::read_mesh(mesh_, _filename, _opt ))
  {
    /// store read option
    opt_ = _opt;
    
    /// update face and vertex normals
    if ( ! opt_.check( IO::Options::FaceNormal ) )
      mesh_.update_face_normals();
    else
      std::cout << "File provides face normals\n";
    
    if ( ! opt_.check( IO::Options::VertexNormal ) )
      mesh_.update_vertex_normals();
    else
      std::cout << "File provides vertex normals\n";


    /// check for possible color information
    if ( opt_.check( IO::Options::VertexColor ) )
    {
      std::cout << "File provides vertex colors\n";
    }
    else
      mesh_.release_vertex_colors();

    if ( _opt.check( IO::Options::FaceColor ) )
    {
      std::cout << "File provides face colors\n";
    }
    else
      mesh_.release_face_colors();

    if ( _opt.check( IO::Options::VertexTexCoord ) )
      std::cout << "File provides texture coordinates\n";

    /// bounding box
    typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin());
    typename Mesh::ConstVertexIter vEnd(mesh_.vertices_end());

    typedef typename Mesh::Point Point;
    using OpenMesh::Vec3f;

    Vec3f bbMin, bbMax;

    bbMin = bbMax = OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt));

    for (size_t count=0; vIt!=vEnd; ++vIt, ++count)
    {
        bbMin.minimize( OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
        bbMax.maximize( OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
    }

    /// set bounding box at the center of the scene
    setSceneBoundingBox(OMVec3f_to_QGLVec(bbMin), OMVec3f_to_QGLVec(bbMax));
    glFogf(GL_FOG_START,1.5*sceneRadius());
    glFogf(GL_FOG_END,  3.0*sceneRadius());
    camera()->showEntireScene();

    /// for normal display
    normal_scale_ = (bbMax-bbMin).min()*0.05f;

    /// info
    std::clog << mesh_.n_vertices() << " vertices, "
          << mesh_.n_edges()    << " edge, "
          << mesh_.n_faces()    << " faces\n";

    /// base point for displaying face normals
    {
      OpenMesh::Utils::Timer t;
      t.start();
      mesh_.add_property( fp_normal_base_ );
      typename M::FaceIter f_it = mesh_.faces_begin();
      typename M::FaceVertexIter fv_it;
      for (;f_it != mesh_.faces_end(); ++f_it)
      {
        typename Mesh::Point v(0,0,0);
        for(fv_it=mesh_.fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
          v += OpenMesh::vector_cast<typename Mesh::Normal>(mesh_.point(*fv_it));
        v *= 1.0f/3.0f;
        mesh_.property( fp_normal_base_, *f_it ) = v;
      }
      t.stop();
      std::clog << "Computed base point for displaying face normals ["
                << t.as_string() << "]" << std::endl;
    }

    /// loading done
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------

template <typename M>
bool TCViewerT<M>::open_texture( const char *_filename )
{
   QImage texsrc;
   QString fname = _filename;

   if (texsrc.load( fname ))
   {      
     return set_texture( texsrc );
   }
   return false;
}


//-----------------------------------------------------------------------------

template <typename M>
bool TCViewerT<M>::set_texture( QImage& _texsrc )
{
  if ( !opt_.vertex_has_texcoord() )
    return false;
   
  {
    /// adjust texture size: 2^k * 2^l
    int tex_w, w( _texsrc.width()  );
    int tex_h, h( _texsrc.height() );

    for (tex_w=1; tex_w <= w; tex_w <<= 1) {};
    for (tex_h=1; tex_h <= h; tex_h <<= 1) {};
    tex_w >>= 1;
    tex_h >>= 1;
    _texsrc = _texsrc.scaled( tex_w, tex_h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
  }

  QImage texture( QGLWidget::convertToGLFormat ( _texsrc ) );
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,   1);
  glPixelStorei(GL_PACK_ROW_LENGTH,    0);
  glPixelStorei(GL_PACK_SKIP_ROWS,     0);
  glPixelStorei(GL_PACK_SKIP_PIXELS,   0);
  glPixelStorei(GL_PACK_ALIGNMENT,     1);    
  
  if ( tex_id_ > 0 )
  {
    glDeleteTextures(1, &tex_id_);
  }
  glGenTextures(1, &tex_id_);
  glBindTexture(GL_TEXTURE_2D, tex_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      
  
  glTexImage2D(GL_TEXTURE_2D,       // target
	       0,                   // level
	       GL_RGBA,             // internal format
	       texture.width(),     // width  (2^n)
	       texture.height(),    // height (2^m)
	       0,                   // border
	       GL_RGBA,             // format
	       GL_UNSIGNED_BYTE,    // type
	       texture.bits() );    // pointer to pixels

  std::cout << "Texture loaded\n";
  return true;
}

//----------------------------------------------------------------------------
template <typename M>
void 
TCViewerT<M>::setDefaultMaterial()
{
    GLfloat mat_a[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat mat_d[] = {0.7, 0.7, 0.5, 1.0};
    GLfloat mat_s[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat shine[] = {120.0};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}


//----------------------------------------------------------------------------
template <typename M>
void
TCViewerT<M>::setDefaultLight()
{
    GLfloat pos1[] = { 0.1,  0.1, -0.02, 0.0};
    GLfloat pos2[] = {-0.1,  0.1, -0.02, 0.0};
    GLfloat pos3[] = { 0.0,  0.0,  0.1,  0.0};
    GLfloat col1[] = { 0.7,  0.7,  0.8,  1.0};
    GLfloat col2[] = { 0.8,  0.7,  0.7,  1.0};
    GLfloat col3[] = { 1.0,  1.0,  1.0,  1.0};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_POSITION, pos1);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,  col1);
    glLightfv(GL_LIGHT0,GL_SPECULAR, col1);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1,GL_POSITION, pos2);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,  col2);
    glLightfv(GL_LIGHT1,GL_SPECULAR, col2);

    glEnable(GL_LIGHT2);
    glLightfv(GL_LIGHT2,GL_POSITION, pos3);
    glLightfv(GL_LIGHT2,GL_DIFFUSE,  col3);
    glLightfv(GL_LIGHT2,GL_SPECULAR, col3);
}

//-----------------------------------------------------------------------------
template <typename M>
void TCViewerT<M>::init()
{
    glDisable(GL_COLOR_MATERIAL);

    /////////////////////////////////////////////////////
    ///       Keyboard shortcut customization         ///
    ///      Changes standard action key bindings     ///
    /////////////////////////////////////////////////////

    /// Define 'Control+Q' as the new exit shortcut (default was 'Escape')
    setShortcut(EXIT_VIEWER, Qt::CTRL+Qt::Key_Q);

    /// add new keyboard event description
    setKeyDescription(Qt::SHIFT+Qt::Key_C, "Toggles GL_CULL_FACE");
    setKeyDescription(Qt::CTRL+Qt::Key_F, "Toggles GL_FOG");

    /// add new mouse binding event description
    setMouseBindingDescription(Qt::ControlModifier, Qt::MiddleButton, "Choose Render Mode", true);

    /// Fog
    GLfloat fogColor[4] = { 0.3, 0.3, 0.4, 1.0 };
    glFogi(GL_FOG_MODE,    GL_LINEAR);
    glFogfv(GL_FOG_COLOR,  fogColor);
    glFogf(GL_FOG_DENSITY, 0.35);
    glHint(GL_FOG_HINT,    GL_DONT_CARE);
    glFogf(GL_FOG_START,    5.0f);
    glFogf(GL_FOG_END,     25.0f);

    /// material and light
    setDefaultMaterial();
    setDefaultLight();

    restoreStateFromFile();
}

template <typename M>
void TCViewerT<M>::keyPressEvent(QKeyEvent *e)
{
    /// Get event modifiers key
    const Qt::KeyboardModifiers modifiers = e->modifiers();

    /// A simple switch on e->key() is not sufficient if we want to take state key into account.
    /// With a switch, it would have been impossible to separate 'F' from 'CTRL+F'.
    /// That's why we use imbricated if...else and a "handled" boolean.
    bool handled = false;
    if ((e->key() == Qt::Key_I) && (modifiers == Qt::NoButton))
    {
        float cp[4];
        glGetLightfv(GL_LIGHT0,GL_POSITION,cp);
        std::cout << "GL_LIGHT0 Position: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT0,GL_DIFFUSE,cp);
        std::cout << "GL_LIGHT0 Diffuse: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT0,GL_SPECULAR,cp);
        std::cout << "GL_LIGHT0 Specular: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;

        glGetLightfv(GL_LIGHT1,GL_POSITION,cp);
        std::cout << "GL_LIGHT1 Position: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT1,GL_DIFFUSE,cp);
        std::cout << "GL_LIGHT1 Diffuse: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT1,GL_SPECULAR,cp);
        std::cout << "GL_LIGHT1 Specular: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;

        glGetLightfv(GL_LIGHT2,GL_POSITION,cp);
        std::cout << "GL_LIGHT2 Position: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT2,GL_DIFFUSE,cp);
        std::cout << "GL_LIGHT2 Diffuse: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;
        glGetLightfv(GL_LIGHT2,GL_SPECULAR,cp);
        std::cout << "GL_LIGHT2 Specular: " << cp[0] << "," << cp[1] << "," << cp[2] << "," << cp[3] << std::endl;

        std::cout << "Scene Radius: " << sceneRadius() << std::endl;
        std::cout << "Scene Center: " << sceneCenter() << std::endl;
    }

    else if ((e->key() == Qt::Key_C) && (modifiers == Qt::ShiftModifier)) {
        if ( glIsEnabled( GL_CULL_FACE ) )
        {
            glDisable( GL_CULL_FACE );
            std::cout << "Back face culling: disabled" << std::endl;
        }
        else
        {
            glEnable( GL_CULL_FACE );
            std::cout << "Back face culling: enabled" << std::endl;
        }
        updateGL();
    }
    else if ((e->key() == Qt::Key_F) && (modifiers == Qt::ControlModifier)) {
        if ( glIsEnabled( GL_FOG ) )
        {
            glDisable( GL_FOG );
            std::cout << "Fog: disabled" << std::endl;
        }
        else
        {
            glEnable( GL_FOG );
            std::cout << "Fog: enabled" << std::endl;
        }
        updateGL();
    }
    else {
    }

    if (!handled)
        QGLViewer::keyPressEvent(e);
}

template <typename M>
void TCViewerT<M>::draw()
{
    glDisable(GL_COLOR_MATERIAL);

    if ( ! mesh_.n_vertices() )
        return;

    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    typename Mesh::ConstFaceVertexIter fvIt;
    
    if (draw_mode_ == "Smooth") {
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

        if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, mesh_.texcoords2D());
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex_id_);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tex_mode_);
        }

        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(*fIt);
            glArrayElement(fvIt->idx());
            ++fvIt;
            glArrayElement(fvIt->idx());
            ++fvIt;
            glArrayElement(fvIt->idx());
        }
        glEnd();

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
        {
            glDisable(GL_TEXTURE_2D);
        }

        setDefaultMaterial();
    } /// "Smooth"

    else if (draw_mode_ == "Flat") {
        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            glNormal3fv( &mesh_.normal(*fIt)[0] );

            fvIt = mesh_.cfv_iter(*fIt);
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
        }
        glEnd();

        setDefaultMaterial();
    } /// "Flat"

    else if (draw_mode_ == "Wireframe") {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(*fIt);
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
        }
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        setDefaultMaterial();
    } /// "Wireframe"

    else if (draw_mode_ == "Points") {
        glDisable(GL_LIGHTING);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

        if (mesh_.has_vertex_colors() && use_color_)
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
        }

        glDrawArrays( GL_POINTS, 0, static_cast<GLsizei>(mesh_.n_vertices()) );
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        setDefaultMaterial();
    } /// "Points"

    else if (draw_mode_ == "Hidden-Line") {
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(*fIt);
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
        }
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glColor4f(0.2f, 0.2f, 0.2f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (fIt=mesh_.faces_begin(); fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(*fIt);
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(*fvIt)[0] );
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        setDefaultMaterial();
    } /// "Hidden-Line"

    else {
        glPushMatrix();

        glScalef(0.6f, 0.6f, 0.6f);

        // cube ///////////////////////////////////////////////////////////////////////
        //    v6----- v5
        //   /|      /|
        //  v1------v0|
        //  | |     | |
        //  | |v7---|-|v4
        //  |/      |/
        //  v2------v3

        GLfloat vertices[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   // v0,v1,v2,v3 (front)
                               1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1,   // v0,v3,v4,v5 (right)
                               1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1,   // v0,v5,v6,v1 (top)
                               -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1,   // v1,v6,v7,v2 (left)
                               -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1,   // v7,v4,v3,v2 (bottom)
                               1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 }; // v4,v7,v6,v5 (back)

        // normal array
        GLfloat normals[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v1,v2,v3 (front)
                               1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // v0,v3,v4,v5 (right)
                               0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // v0,v5,v6,v1 (top)
                               -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // v1,v6,v7,v2 (left)
                               0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // v7,v4,v3,v2 (bottom)
                               0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1 }; // v4,v7,v6,v5 (back)

        // color array
        GLfloat colors[]   = { 1, 1, 1,   1, 1, 0,   1, 0, 0,   1, 0, 1,   // v0,v1,v2,v3 (front)
                               1, 1, 1,   1, 0, 1,   0, 0, 1,   0, 1, 1,   // v0,v3,v4,v5 (right)
                               1, 1, 1,   0, 1, 1,   0, 1, 0,   1, 1, 0,   // v0,v5,v6,v1 (top)
                               1, 1, 0,   0, 1, 0,   0, 0, 0,   1, 0, 0,   // v1,v6,v7,v2 (left)
                               0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,   // v7,v4,v3,v2 (bottom)
                               0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1 }; // v4,v7,v6,v5 (back)


        // index array of vertex array for glDrawElements() & glDrawRangeElement()
        GLubyte indices[]  = { 0, 1, 2,   2, 3, 0,      // front
                               4, 5, 6,   6, 7, 4,      // right
                               8, 9,10,  10,11, 8,      // top
                               12,13,14,  14,15,12,      // left
                               16,17,18,  18,19,16,      // bottom
                               20,21,22,  22,23,20 };    // back

        // enable and specify pointers to vertex arrays
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glNormalPointer(GL_FLOAT, 0, normals);
        glColorPointer(3, GL_FLOAT, 0, colors);
        glVertexPointer(3, GL_FLOAT, 0, vertices);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        glPopMatrix();
    } /// default box for debugging
}

template <typename M>
QString TCViewerT<M>::helpString() const
{
    return 0;
}

template <typename M>
void TCViewerT<M>::set_draw_mode(const string _mode)
{
    draw_mode_ = _mode;
}
