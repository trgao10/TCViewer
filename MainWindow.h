#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class TCViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public:
    void createActions(TCViewer *viewer);
    void createMenus();

    QMenu *fileMenu;
    QMenu *renderMenu;
    QMenu *helpMenu;
    QActionGroup *renderModeGroup;
    QAction *openAct;
    QAction *texAct;
    QAction *exitAct;
    QAction *SmoothAct;
    QAction *FlatAct;
    QAction *WireframeAct;
    QAction *PointsAct;
    QAction *HiddenLineAct;
    QAction *ValenceAct;
    QAction *GaussianCurvatureAct;
    QAction *MeanCurvatureAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QLabel *infoLabel;

protected:
    virtual void mousePressEvent (QMouseEvent * event);
};

#endif
