#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int sendTurn = 0;
int motorStatus = 0;
int motorSpeed = 140;
int motorAnem = 10;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    openCANPort();
    timer_tick = new QTimer();
    timer_OpenGL = new QTimer();
    ui->setupUi(this);
    this->setFixedSize(this->size());

    //timer for opengl
    connect(timer_OpenGL, SIGNAL(timeout()), this, SLOT(onTimer_OpenGL()));
    timer_OpenGL -> start (50); //20 ms

    connect( timer_tick, SIGNAL(timeout()), this, SLOT(onTimer_Tick()));
    timer_tick -> start(1); // in ms

    connect(ui->button_motorStatus,SIGNAL(clicked()),this,SLOT(onMotorButtonClicked()));
    ui->slider_motorSpeed->setRange(50,200);
    ui->slider_motorSpeed->setValue(140);
    ui->value_motorSpeed->setText("140");

    connect(ui->slider_motorSpeed, SIGNAL(valueChanged(int)), SLOT(motorSpeedChanged(int)));
    ui->slider_anemTrigger->setRange(1,20);
    ui->slider_anemTrigger->setValue(10);
    ui->value_anemTrigger->setText("10");
    connect(ui->slider_anemTrigger, SIGNAL(valueChanged(int)), SLOT(motorAnemChanged(int)));

    timer_tick -> start(1); // in ms
    connect(ui->bouton_switch,SIGNAL(clicked()),this,SLOT(onButSendClicked()));
    initGL();
    sendCANMotorMessage(); // to set the state of the motor

}

void MainWindow::initGL()
{
    Object_GL = new ObjectOpenGL(ui->glW);
    Object_GL->setObjectName(QString::fromUtf8("ObjectOpenGrL"));
    Object_GL->setGeometry(QRect(0, 0, 371, 251));

    QMenu *FileMenu = menuBar()->addMenu("&File");
    FileMenu->addSeparator();
    FileMenu->addAction("Quit", qApp, SLOT (quit()), QKeySequence(tr("Ctrl+Q")));

    // Add menu items
    QMenu *ViewMenu = menuBar()->addMenu("&View");
    ViewMenu->addAction("Front view",       Object_GL, SLOT (FrontView()),  QKeySequence(tr("Ctrl+f")));
    ViewMenu->addAction("Rear view",        Object_GL, SLOT (RearView()),   QKeySequence(tr("Ctrl+e")));
    ViewMenu->addAction("Left view",        Object_GL, SLOT (LeftView()),   QKeySequence(tr("Ctrl+l")));
    ViewMenu->addAction("Right view",       Object_GL, SLOT (RightView()),  QKeySequence(tr("Ctrl+r")));
    ViewMenu->addAction("Top view",         Object_GL, SLOT (TopView()),    QKeySequence(tr("Ctrl+t")));
    ViewMenu->addAction("Bottom view",      Object_GL, SLOT (BottomView()), QKeySequence(tr("Ctrl+b")));
    FileMenu->addSeparator();
    ViewMenu->addAction("Isometric",        Object_GL, SLOT (IsometricView()), QKeySequence(tr("Ctrl+i")));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openCANPort()
{
    if (socket_can.open("can0") == scpp::STATUS_OK)
    {
         printf("can socket opened");
    }
    else
    {
        printf("Cannot open can socket!");
    }
}
void MainWindow::receiveCANMessage()
{

 scpp::CanFrame fr;

    if(socket_can.read(fr) == scpp::STATUS_OK)
	{

        switch (fr.id) {
            case 0x01: { // temperature from accelerometer
                int16_t temperature = (int16_t)((fr.data[1] << 8) | fr.data[0]);
//                ui->value_lpstemp->display(QString::number(temperature));
                break;
            }
            case 0x09 : { // Luminosite
                int16_t value = (int16_t)((fr.data[1] << 8) | fr.data[0]);
                ui->display_VL6180X->display(value);
                ui->mesure_unit->setText("lux");
                ui->lbl_mode->setText("Luminosite");
                break;
            }
            case 0x05 : { // Distance
                int16_t value = (int16_t)((fr.data[1] << 8) | fr.data[0]);
                ui->display_VL6180X->display(value);
                ui->mesure_unit->setText("mm");
                ui->lbl_mode->setText("Distance");
                break;
            }
            case 0x06: { // temperature
                int16_t temperature = (int16_t)((fr.data[1] << 8) | fr.data[0]);
                temperature-=240;
                ui->value_htstemp->display(QString::number(temperature));
                break;
            }
            case 0x07: { // pression
                int32_t pression = (fr.data[0] << 24) | (fr.data[1] << 16) | (fr.data[2] << 8) | fr.data[3];
                ui->value_pression->display(QString::number(pression));
                break;
            }
            case 0x08: {
                int16_t humidite = (int16_t)((fr.data[1] << 8) | fr.data[0]);
                humidite/=10.0;
                ui->value_humi->display(QString::number(humidite));
                break;
            }

            case 0x55 :{
                ui->value_anem->display(QString::number((int16_t)fr.data[0]));
                break;
            }
        }


	}
}

void MainWindow::onTimer_Tick()
{
    receiveCANMessage();
}

void MainWindow::onTimer_OpenGL()
{
    receiveOpenGL();
}

void MainWindow::receiveOpenGL()
{
     scpp::CanFrame fr;

        if(socket_can.read(fr) == scpp::STATUS_OK)
        {
            switch (fr.id ){
                case 0x01: {

                    for(int i=0;i<3;i++)
                    {
                        acc[i] = ((double) ((int16_t) (fr.data[i*2+1]<<8 | fr.data[i*2]))) * (4.F/32768.F);
                    }
                    // calibration
//                    acc[0] -= 0.12;
//                    acc[1] += 3.3;


                    ui->lblMPAccX->setText(QString("%1 G").arg(acc[0], 0, 'g', 2));
                    ui->lblMPAccY->setText(QString("%1 G").arg(acc[1], 0, 'g', 2));
                    ui->lblMPAccZ->setText(QString("%1 G").arg(acc[2], 0, 'g', 2));
                    break;
                }
                case 0x02 :{
                    for(int i=0;i<3;i++)
                    {
                        gyr[i] = ((double) ((int16_t) (fr.data[i*2+1]<<8 | fr.data[i*2]))) * (1000.F/32768.F)*(M_PI/180.F);
                    }
                    gyr[2] -= 3.8;
                    for (int i = 0; i< 3; i++){
                        if(abs(gyr[i]) < 0.1) gyr[i] = 0 ;
                    }
                    ui->lblMPGyrY->setText(QString("%1 rad/s").arg(gyr[1], 0, 'g', 2));
                    ui->lblMPGyrZ->setText(QString("%1 rad/s").arg(gyr[2], 0, 'g', 2));
                    ui->lblMPGyrX->setText(QString("%1 rad/s").arg(gyr[0], 0, 'g', 2));
                    break;
                }

            }
        }
 updateGL();
}
void MainWindow::updateGL()
{   // Magdwick et calcul quaternions
    MadgwickAHRSupdateIMU(gyr[0], gyr[1], gyr[2], acc[0], acc[1], acc[2]);
    double R12=2*(q1*q2+q0*q3);
    double R22=q0*q0+q1*q1-q2*q2-q3*q3;
    double R31=2*(q0*q1+q2*q3);
    double R32=2*(q1*q3-q0*q2);
    double R33=q0*q0-q1*q1-q2*q2+q3*q3;

    double phi = atan2(R31,R33)*720.F/M_PI;
    double theta = -asin(R32)*720.F/M_PI;
    double psi = atan2(R12,R22)*720.F/M_PI;
    ui->lblMPphi->setText(QString("%1").arg(phi, 0, 'g', 3));
    ui->lblMPtheta->setText(QString("%1").arg(theta, 0, 'g', 3));
    ui->lblMPpsi->setText(QString("%1").arg(psi, 0, 'g', 3));
    Object_GL->setAngles(theta, phi, psi);
    Object_GL->update();
}

void MainWindow::onButSendClicked()
{
    sendCANMessage();
}

//Start
void MainWindow::sendCANMessage()
{

    scpp::CanFrame frame_to_write;

    frame_to_write.id     =   0x10;
    frame_to_write.len     =   1;

    frame_to_write.data[0] = 'h';


    socket_can.write(frame_to_write);
}

void MainWindow::sendCANMotorMessage()
{

    scpp::CanFrame frame_to_write;

    frame_to_write.id     =   0x04;
    frame_to_write.len     =   8;
    frame_to_write.data[0] = (int)motorStatus;
    frame_to_write.data[1] = (int)motorSpeed;
    frame_to_write.data[2] = (int)motorAnem;
    frame_to_write.data[3] = 0;
    frame_to_write.data[4] = 0;
    frame_to_write.data[5] = 0;
    frame_to_write.data[6] = 0;
    frame_to_write.data[7] = 0;

    socket_can.write(frame_to_write);
}

void MainWindow::onMotorButtonClicked()
{

    motorStatus += 1;
    motorStatus %= 3;
    if (motorStatus == 0){
        ui->button_motorStatus->setText("State 0");
    }
    else if (motorStatus == 1){
        ui->button_motorStatus->setText("State 1");
    }else if (motorStatus == 2){
        ui->button_motorStatus->setText("State 2");
    }
    sendCANMotorMessage();

}

void MainWindow::motorSpeedChanged(int value)
{
    motorSpeed = value;
    ui->value_motorSpeed->setText(QString::number(motorSpeed));
    sendCANMotorMessage();
}

void MainWindow::motorAnemChanged(int value)
{
    motorAnem = value;
    ui->value_anemTrigger->setText(QString::number(motorAnem));
    sendCANMotorMessage();
}
