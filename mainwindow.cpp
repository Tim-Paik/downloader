#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Downloader down;
    //connect(&down,SIGNAL(downloadComplete()),this,SLOT(ok()));
    connect(&down,&Downloader::downloadComplete,this,&MainWindow::ok);
    qDebug()<<"connect ok";
    qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();
    QDateTime time = QDateTime::currentDateTime();
    qDebug()<<time.toString("yyyy-MM-dd hh:mm:ss ddd");
    down.start("https://mirrors.tuna.tsinghua.edu.cn/archlinuxcn/x86_64/archlinuxcn.files");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ok() {
    ui->label->setText("OK!");
}
