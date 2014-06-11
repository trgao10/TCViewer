 #ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>

 class QAction;
 class QActionGroup;
 class QLabel;
 class QMenu;

 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow();

 protected:
     void contextMenuEvent(QContextMenuEvent *event);

 private slots:
     void newFile();
     void open();
     void save();
     void print();
     void undo();
     void redo();
     void cut();
     void copy();
     void paste();
     void bold();
     void italic();
     void leftAlign();
     void rightAlign();
     void justify();
     void center();
     void setLineSpacing();
     void setParagraphSpacing();
     void about();
     void aboutQt();

 public:
     void createActions();
     void createMenus();

     QMenu *fileMenu;
     QMenu *editMenu;
     QMenu *formatMenu;
     QMenu *helpMenu;
     QActionGroup *alignmentGroup;
     QAction *newAct;
     QAction *openAct;
     QAction *texAct;
     QAction *saveAct;
     QAction *printAct;
     QAction *exitAct;
     QAction *undoAct;
     QAction *redoAct;
     QAction *cutAct;
     QAction *copyAct;
     QAction *pasteAct;
     QAction *boldAct;
     QAction *italicAct;
     QAction *leftAlignAct;
     QAction *rightAlignAct;
     QAction *justifyAct;
     QAction *centerAct;
     QAction *setLineSpacingAct;
     QAction *setParagraphSpacingAct;
     QAction *aboutAct;
     QAction *aboutQtAct;
     QLabel *infoLabel;
 };

 #endif
