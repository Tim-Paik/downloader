#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Downloader down;
    connect(&down,SIGNAL(downloadComplete()),this,SLOT(ok()));
    qDebug()<<"connect ok";
    qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();
    QDateTime time = QDateTime::currentDateTime();
    qDebug()<<time.toString("yyyy-MM-dd hh:mm:ss ddd");
    down.start("https://home.timpaik.top:8008/100MiB.file");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ok() {
    ui->label->setText("OK!");
}
