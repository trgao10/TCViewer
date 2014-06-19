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
void
TCViewerT<M>::setDefaultMaterial() {
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
TCViewerT<M>::setDefaultLight() {
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
void TCViewerT<M>::init() {}

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
void TCViewerT<M>::draw() {}

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

template <typename M>
void TCViewerT<M>::postDraw()
{
    QGLViewer::postDraw();
    setDefaultMaterial();
}
