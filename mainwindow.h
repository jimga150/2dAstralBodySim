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

    void updateBodyCount(int numBodies);

private slots:
    void on_radiusSpinBox_valueChanged(double arg1);
    void on_pauseplayButton_clicked();
    void on_trailsCheckBox_stateChanged(int arg1);
    void on_clearButton_clicked();
    void on_trailTLenSpinBox_valueChanged(double arg1);
    void on_resetViewButton_clicked();
};

#endif // MainWindow_H
