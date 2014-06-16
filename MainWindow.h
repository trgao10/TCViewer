#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

//private slots:
//    void Smooth();
//    void Flat();
//    void Wireframe();
//    void Points();
//    void HiddenLine();
//    void about();
//    void aboutQt();

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
    QAction *aboutAct;
    QAction *aboutQtAct;
    QLabel *infoLabel;
};

#endif
