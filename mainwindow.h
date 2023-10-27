#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "socketcan_cpp.h"

//Johnny
#include <QtCore>
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <QtOpenGL/QGLWidget> // widget : fenetre, bouton, ....
#include <QThread>
#include "madgwickahrs.h" // code pour les angles d'EUler
#include "objectgl.h" // orientation de carte graphiquement
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <fcntl.h>    // O_RDWR
#include <signal.h>
#include <unistd.h>   // exit

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openCANPort();
    void sendCANMessage();
    void receiveCANMessage();
    void receiveOpenGL();
    void sendCANMotorMessage();


public slots:
    void onTimer_Tick();
    void onButSendClicked();
    void onTimer_OpenGL();

    void onMotorButtonClicked();
    void motorSpeedChanged(int value);
    void motorAnemChanged(int value);

private:
    scpp::SocketCan socket_can;
    Ui::MainWindow *ui;
    QTimer *timer_tick;
    QTimer *timer_OpenGL;

    ObjectOpenGL *Object_GL; // instanciation de notre objet 3D
    void sendOrder(int id, qint8 order); // ordre de priorite
    void initGL(); // initialisation objet 3D
    void updateGL(); // update objet 3D
    double acc[3], gyr[3]; // x,y et z
};
#endif // MAINWINDOW_H
