#include <QtGui>
#include <iostream>

#include "MainWindow.h"
#include "TCViewer.h"

MainWindow::MainWindow()
{
}

void MainWindow::createActions(TCViewer* viewer)
{
    openAct= new QAction(tr("&Open Mesh..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open a mesh file"));
    connect(openAct, SIGNAL(triggered()), viewer, SLOT(query_open_mesh_file()));

    texAct = new QAction(tr("Open Texture..."), this);
    texAct->setShortcut(tr("Ctrl+T"));
    texAct->setStatusTip(tr("Open a texture file"));
    connect(texAct, SIGNAL(triggered()), viewer, SLOT(query_open_texture_file()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), viewer, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(aboutQtAct, SIGNAL(triggered()), viewer, SLOT(aboutQt()));

    SmoothAct = new QAction(tr("&Smooth"), this);
    SmoothAct->setCheckable(true);
    SmoothAct->setShortcut(tr("Shift+S"));
    SmoothAct->setStatusTip(tr("Smooth Shading"));
    connect(SmoothAct, SIGNAL(triggered()), viewer, SLOT(Smooth()));

    FlatAct = new QAction(tr("&Flat"), this);
    FlatAct->setCheckable(true);
    FlatAct->setShortcut(tr("Shift+F"));
    FlatAct->setStatusTip(tr("Flat Shading"));
    connect(FlatAct, SIGNAL(triggered()), viewer, SLOT(Flat()));

    WireframeAct = new QAction(tr("&Wireframe"), this);
    WireframeAct->setCheckable(true);
    WireframeAct->setShortcut(tr("Shift+W"));
    WireframeAct->setStatusTip(tr("Display Wireframe"));
    connect(WireframeAct, SIGNAL(triggered()), viewer, SLOT(Wireframe()));

    PointsAct = new QAction(tr("&Points"), this);
    PointsAct->setCheckable(true);
    PointsAct->setShortcut(tr("Shift+P"));
    PointsAct->setStatusTip(tr("Display Points"));
    connect(PointsAct, SIGNAL(triggered()), viewer, SLOT(Points()));

    HiddenLineAct = new QAction(tr("&Hidden-Line"), this);
    HiddenLineAct->setCheckable(true);
    HiddenLineAct->setShortcut(tr("Shift+H"));
    HiddenLineAct->setStatusTip(tr("Hidden-Line"));
    connect(HiddenLineAct, SIGNAL(triggered()), viewer, SLOT(HiddenLine()));

    ValenceAct = new QAction(tr("&Valence"), this);
    ValenceAct->setCheckable(true);
    ValenceAct->setShortcut(tr("Shift+V"));
    ValenceAct->setStatusTip(tr("View Vertex Valence"));
    connect(ValenceAct, SIGNAL(triggered()), viewer, SLOT(Valence()));

    GaussianCurvatureAct = new QAction(tr("&Gaussian Curvature"), this);
    GaussianCurvatureAct->setCheckable(true);
    GaussianCurvatureAct->setShortcut(tr("Shift+G"));
    GaussianCurvatureAct->setStatusTip(tr("View Gaussian Curvature"));
    connect(GaussianCurvatureAct, SIGNAL(triggered()), viewer, SLOT(GaussianCurvature()));

    MeanCurvatureAct = new QAction(tr("&Mean Curvature"), this);
    MeanCurvatureAct->setCheckable(true);
    MeanCurvatureAct->setShortcut(tr("Shift+M"));
    MeanCurvatureAct->setStatusTip(tr("View Mean Curvature"));
    connect(MeanCurvatureAct, SIGNAL(triggered()), viewer, SLOT(MeanCurvature()));

    renderModeGroup = new QActionGroup(this);
    renderModeGroup->addAction(SmoothAct);
    renderModeGroup->addAction(FlatAct);
    renderModeGroup->addAction(WireframeAct);
    renderModeGroup->addAction(PointsAct);
    renderModeGroup->addAction(HiddenLineAct);
    renderModeGroup->addAction(ValenceAct);
    renderModeGroup->addAction(GaussianCurvatureAct);
    renderModeGroup->addAction(MeanCurvatureAct);
    SmoothAct->setChecked(true);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(texAct);

    renderMenu = menuBar()->addMenu(tr("&Render"));
    renderMenu->addAction(SmoothAct);
    renderMenu->addAction(FlatAct);
    renderMenu->addAction(WireframeAct);
    renderMenu->addAction(PointsAct);
    renderMenu->addAction(HiddenLineAct);
    renderMenu->addAction(ValenceAct);
    renderMenu->addAction(GaussianCurvatureAct);
    renderMenu->addAction(MeanCurvatureAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() == Qt::MidButton) && (event->modifiers() == Qt::ControlModifier))
    {
        QMenu menu(this);
        menu.addAction(SmoothAct);
        menu.addAction(FlatAct);
        menu.addAction(WireframeAct);
        menu.addAction(PointsAct);
        menu.addAction(HiddenLineAct);
        menu.addAction(ValenceAct);
        menu.addAction(GaussianCurvatureAct);
        menu.addAction(MeanCurvatureAct);
        menu.exec(event->globalPos());
    }
    else {
        QWidget::mousePressEvent(event);
    }
}

