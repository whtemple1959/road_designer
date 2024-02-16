#include "mainwindow.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent)
{

    m_pViewer = new Viewer(this, width(), height(), "objs/gc_height_257.png", "objs/gc_texture_256.png");

    setMinimumSize(640, 480);
    setCentralWidget(m_pViewer);

}

MainWindow::~MainWindow()
{

}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
    m_pViewer->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent * e)
{
    m_pViewer->keyReleaseEvent(e);
}
