#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QRandomGenerator>

#include "abswindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ABSWindow simwindow;

private:
    Ui::MainWindow *ui;

public slots:

    void makedisk();

};

#endif // MainWindow_H
