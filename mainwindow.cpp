#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);

    this->simwindow.setFormat(format);

    QWidget* windowcontainer = QWidget::createWindowContainer(&this->simwindow, this);
    this->ui->windowWidget->addWidget(windowcontainer);

    this->ui->radiusSpinBox->setValue(this->simwindow.default_body_radius_m);

    this->simwindow.show();
    this->simwindow.setAnimating(true);

    connect(this->ui->actionAccretion_Disk, &QAction::triggered, this, &MainWindow::makedisk);

    this->ui->trailsCheckBox->setChecked(this->simwindow.enable_trails);

    connect(&this->simwindow, &ABSWindow::numBodiesChanged, this, &MainWindow::updateBodyCount);

    this->on_trailTLenSpinBox_valueChanged(this->ui->trailTLenSpinBox->value());

    this->ui->gravFieldLowRadioButton->toggle();

}

MainWindow::~MainWindow(){
    delete ui;
    this->simwindow.deleteLater();
}

void MainWindow::makedisk(){

    QRandomGenerator rng = QRandomGenerator::securelySeeded();

    QSize simwindow_size = this->simwindow.window_size;
    int smaller_window_dim = simwindow_size.height();
    if (simwindow_size.width() < simwindow_size.height()){
        smaller_window_dim = simwindow_size.width();
    }

    float disk_radius = (smaller_window_dim/this->simwindow.viewscale_p_m)/2.0;

    float bigbody_radius = disk_radius/10;
    float bigbody_mass = M_PI*bigbody_radius*bigbody_radius*this->simwindow.fixturedef_template.density;

    this->simwindow.createBody(bigbody_radius, this->simwindow.viewcenter_m, b2Vec2(0, 0));

    for (int i = 0; i < 100; ++i){

        float dist = disk_radius*(rng.generateDouble()*0.7 + 0.3);
        float angle = i*2*M_PI/100.0;

        float x = dist*cos(angle);
        float y = dist*sin(angle);
        b2Vec2 pos(x, y);

        float smallbody_radius = bigbody_radius*(rng.generateDouble()*0.05 + 0.05);

        float velocity_mag = sqrt(this->simwindow.big_G*bigbody_mass/dist);
        float v_x = velocity_mag*cos(angle + M_PI/2);
        float v_y = velocity_mag*sin(angle + M_PI/2);
        b2Vec2 vel(v_x, v_y);

        this->simwindow.createBody(smallbody_radius, pos + this->simwindow.viewcenter_m, vel);
    }

}

void MainWindow::updateBodyCount(int numBodies){
    this->ui->statusbar->showMessage(QString("Number of bodies: ") + QString::number(numBodies));
}

void MainWindow::on_radiusSpinBox_valueChanged(double arg1){
    this->simwindow.default_body_radius_m = arg1;
}


void MainWindow::on_pauseplayButton_clicked(){
    this->simwindow.paused = !this->simwindow.paused;
    if (this->simwindow.paused){
        this->ui->pauseplayButton->setText("Resume");
    } else {
        this->ui->pauseplayButton->setText("Pause");
    }
}


void MainWindow::on_trailsCheckBox_stateChanged(int arg1){
    if (arg1 == Qt::Unchecked){
        this->simwindow.enable_trails = false;
    } else {
        this->simwindow.enable_trails = true;
    }
}


void MainWindow::on_clearButton_clicked(){
    for (b2Body* b = this->simwindow.world->GetBodyList(); b; b = b->GetNext()){
        this->simwindow.destroyBody(b);
    }
}


void MainWindow::on_trailTLenSpinBox_valueChanged(double arg1){
    int num_frames = arg1/this->simwindow.timeStep_s;
    this->simwindow.max_position_hist_entries = num_frames;
}


void MainWindow::on_resetViewButton_clicked(){
    this->simwindow.viewcenter_m = this->simwindow.viewcenter_m_default;
    this->simwindow.viewscale_p_m = this->simwindow.viewscale_p_m_default;
}


void MainWindow::on_gravFieldCheckBox_stateChanged(int arg1){
    if (arg1 == Qt::Unchecked){
        this->simwindow.enable_gravfield = false;
    } else {
        this->simwindow.enable_gravfield = true;
    }
}


void MainWindow::on_gravFieldHighRadioButton_toggled(bool checked){
    if (checked) this->simwindow.gravfield_rowscols = 50;
}


void MainWindow::on_gravFieldMedRadioButton_toggled(bool checked){
    if (checked) this->simwindow.gravfield_rowscols = 20;
}


void MainWindow::on_gravFieldLowRadioButton_toggled(bool checked){
    if (checked) this->simwindow.gravfield_rowscols = 10;
}

