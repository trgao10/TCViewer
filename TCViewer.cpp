#include "TCViewer.h"

///-----------------------------------------------------------------------------
/// load mesh and texture
///-----------------------------------------------------------------------------
bool TCViewer::open_mesh(const char* _filename, IO::Options _opt)
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
        OpenMesh::Utils::Timer t;
        t.start();
        mesh_.add_property( fp_normal_base_ );
        TCMesh::FaceIter f_it = mesh_.faces_begin();
        TCMesh::FaceVertexIter fv_it;
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

        /// compute vertex valence and convert them to "valence_color"'s
        std::cout << "Computing vertex valences..." << std::endl;
        int valence;
        std::vector<float>  valences;
        for (vIt=mesh_.vertices_begin(); vIt!=vEnd; ++vIt) {
            valence = 0;
            for (TCMesh::VertexVertexIter vvIt=mesh_.vv_iter(*vIt); vvIt.is_valid(); ++vvIt) {
                valence++;
            }
            mesh_.data(*vIt).set_valence(valence);
            valences.push_back(valence);
        }

        float range_min = *std::max_element(valences.begin(),valences.end());
        float range_max = *std::min_element(valences.begin(),valences.end());

        Vec3f valence_color;
        for (vIt=mesh_.vertices_begin(); vIt!=vEnd; ++vIt) {
            valence_color = interp_color(mesh_.data(*vIt).get_valence(), range_min, range_max);
            mesh_.data(*vIt).set_valence_color(valence_color);
        }
        std::cout << "Valence computation done." << std::endl;

        /// compute Gaussian and mean curvatures and convert them to corresponding colors


        /// loading done
        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
bool TCViewer::open_texture( const char *_filename )
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
bool TCViewer::set_texture( QImage& _texsrc )
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

///-----------------------------------------------------------------------------
/// GUI functions
///-----------------------------------------------------------------------------
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

///-----------------------------------------------------------------------------
/// reload draw(), init()
///-----------------------------------------------------------------------------
void TCViewer::draw()
{
    if ( ! mesh_.n_vertices() )
        return;

    glDisable(GL_COLOR_MATERIAL);

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
    else if (draw_mode_ == "Valence") {
        mesh_.request_vertex_colors();
        TCMesh::VertexIter vIt, vEnd(mesh_.vertices_end());
        for (vIt=mesh_.vertices_begin(); vIt!=vEnd; ++vIt) {
            mesh_.set_color(*vIt, TCMesh::Color(mesh_.data(*vIt).get_valence_color()));
        }

        glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

        if ( mesh_.has_vertex_colors() )
        {
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
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
        glDisableClientState(GL_COLOR_ARRAY);
    } /// "Valence"
    else if (draw_mode_ == "GaussianCurvature") {
        mesh_.request_vertex_colors();
        TCMesh::VertexIter vIt, vEnd(mesh_.vertices_end());
        for (vIt=mesh_.vertices_begin(); vIt!=vEnd; ++vIt) {
            mesh_.set_color(*vIt, TCMesh::Color(mesh_.data(*vIt).get_valence_color()));
        }

        glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

        if ( mesh_.has_vertex_colors() )
        {
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
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
        glDisableClientState(GL_COLOR_ARRAY);
    } /// "GaussianCurvature"
    else if (draw_mode_ == "MeanCurvature") {
        mesh_.request_vertex_colors();
        TCMesh::VertexIter vIt, vEnd(mesh_.vertices_end());
        for (vIt=mesh_.vertices_begin(); vIt!=vEnd; ++vIt) {
            mesh_.set_color(*vIt, TCMesh::Color(mesh_.data(*vIt).get_valence_color()));
        }

        glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

        if ( mesh_.has_vertex_colors() )
        {
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
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
        glDisableClientState(GL_COLOR_ARRAY);
    } /// "MeanCurvature"
    else {
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
        //        glPushMatrix();

        //        glScalef(0.6f, 0.6f, 0.6f);

        //        // cube ///////////////////////////////////////////////////////////////////////
        //        //    v6----- v5
        //        //   /|      /|
        //        //  v1------v0|
        //        //  | |     | |
        //        //  | |v7---|-|v4
        //        //  |/      |/
        //        //  v2------v3

        //        GLfloat vertices[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   // v0,v1,v2,v3 (front)
        //                               1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1,   // v0,v3,v4,v5 (right)
        //                               1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1,   // v0,v5,v6,v1 (top)
        //                               -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1,   // v1,v6,v7,v2 (left)
        //                               -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1,   // v7,v4,v3,v2 (bottom)
        //                               1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 }; // v4,v7,v6,v5 (back)

        //        // normal array
        //        GLfloat normals[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v1,v2,v3 (front)
        //                               1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // v0,v3,v4,v5 (right)
        //                               0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // v0,v5,v6,v1 (top)
        //                               -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // v1,v6,v7,v2 (left)
        //                               0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // v7,v4,v3,v2 (bottom)
        //                               0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1 }; // v4,v7,v6,v5 (back)

        //        // color array
        //        GLfloat colors[]   = { 1, 1, 1,   1, 1, 0,   1, 0, 0,   1, 0, 1,   // v0,v1,v2,v3 (front)
        //                               1, 1, 1,   1, 0, 1,   0, 0, 1,   0, 1, 1,   // v0,v3,v4,v5 (right)
        //                               1, 1, 1,   0, 1, 1,   0, 1, 0,   1, 1, 0,   // v0,v5,v6,v1 (top)
        //                               1, 1, 0,   0, 1, 0,   0, 0, 0,   1, 0, 0,   // v1,v6,v7,v2 (left)
        //                               0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,   // v7,v4,v3,v2 (bottom)
        //                               0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1 }; // v4,v7,v6,v5 (back)


        //        // index array of vertex array for glDrawElements() & glDrawRangeElement()
        //        GLubyte indices[]  = { 0, 1, 2,   2, 3, 0,      // front
        //                               4, 5, 6,   6, 7, 4,      // right
        //                               8, 9,10,  10,11, 8,      // top
        //                               12,13,14,  14,15,12,      // left
        //                               16,17,18,  18,19,16,      // bottom
        //                               20,21,22,  22,23,20 };    // back

        //        // enable and specify pointers to vertex arrays
        //        glEnableClientState(GL_NORMAL_ARRAY);
        //        glEnableClientState(GL_COLOR_ARRAY);
        //        glEnableClientState(GL_VERTEX_ARRAY);
        //        glNormalPointer(GL_FLOAT, 0, normals);
        //        glColorPointer(3, GL_FLOAT, 0, colors);
        //        glVertexPointer(3, GL_FLOAT, 0, vertices);

        //        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

        //        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        //        glDisableClientState(GL_COLOR_ARRAY);
        //        glDisableClientState(GL_NORMAL_ARRAY);

        //        glPopMatrix();
    } /// default smooth shading
}

void TCViewer::init() {
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


///-----------------------------------------------------------------------------
/// Utilities
///-----------------------------------------------------------------------------
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
}

void TCViewer::Valence()
{
    set_draw_mode("Valence");
    updateGL();
}

void TCViewer::GaussianCurvature()
{
    std::cout << "Gaussian Curvature!" << std::endl;
    set_draw_mode("GaussianCurvature");
    updateGL();
}

void TCViewer::MeanCurvature()
{
    std::cout << "Mean Curvature!" << std::endl;
    set_draw_mode("MeanCurvature");
    updateGL();
}

void TCViewer::about()
{
    help();
}

void TCViewer::aboutQt()
{
}

OpenMesh::Vec3f TCViewer::interp_color(float _val)
{
    int _ival = (int)_val;

    /// red
    int red = _ival * _ival * _ival * 42 % 255;

    /// green
    int green = _ival * _ival * 42 % 255;

    /// blue
    int blue = _ival * 42 % 255;


    OpenMesh::Vec3f interped_val(red, green, blue);

    return interped_val;
}

OpenMesh::Vec3f TCViewer::interp_color(float _val, float range_min, float range_max)
{
    OpenMesh::Vec3f red(1,0,0), green(0,1,0), blue(0,0,1);

    OpenMesh::Vec3f interped_val = green;

    if (range_min==range_max) {
        return interped_val*255;
    }
    else {
        _val = (_val-range_min)/(range_max-range_min);
        if (_val <= 0.5) {
            interped_val  = (1.0f-2.0f*_val)*red+2.0f*_val*green;
        }
        else {
            interped_val = (2.0f-2.0f*_val)*green+(2.0f*_val-1.0f)*blue;
        }
    }

    return interped_val*255;
}
