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

    QList<QAction*> preset_actions = this->ui->menuPresets->actions();

    for (QAction* action : preset_actions){
        if (action->text() == QString("Accretion Disk")){
            connect(action, &QAction::triggered, this, &MainWindow::makedisk);
        }
    }


}

MainWindow::~MainWindow()
{
    delete ui;
    this->simwindow.deleteLater();
}

void MainWindow::makedisk(){

    QRandomGenerator rng = QRandomGenerator::securelySeeded();

    float bigbody_radius = 1;
    float bigbody_mass = M_PI*bigbody_radius*bigbody_radius*this->simwindow.fixturedef_template.density;

    this->simwindow.createBody(bigbody_radius, this->simwindow.viewcenter_m, b2Vec2(0, 0));

    for (int i = 0; i < 100; ++i){

        float dist = bigbody_radius*(rng.generateDouble()*12 + 8);
        float angle = i*2*M_PI/100.0;

        float x = dist*cos(angle);
        float y = dist*sin(angle);
        b2Vec2 pos(x, y);

        float radius = rng.generateDouble()*0.05 + 0.05;

        float velocity_mag = sqrt(this->simwindow.big_G*bigbody_mass/dist);
        float v_x = velocity_mag*cos(angle + M_PI/2);
        float v_y = velocity_mag*sin(angle + M_PI/2);
        b2Vec2 vel(v_x, v_y);

        this->simwindow.createBody(radius, pos + this->simwindow.viewcenter_m, vel);
    }

}
