#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);

    this->simwindow.setFormat(format);

    QWidget* windowcontainer = QWidget::createWindowContainer(&this->simwindow, this);
    this->ui->centralwidget->layout()->addWidget(windowcontainer);

    this->simwindow.show();
    this->simwindow.setAnimating(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    this->simwindow.deleteLater();
}
