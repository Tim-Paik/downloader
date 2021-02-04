#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString();
    qDebug()<<"OpenSSL support: "<< QSslSocket::supportsSsl();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ok() {
    ui->label->setText("Download Complete!");
}

void MainWindow::on_pushButton_clicked()
{
    downloader = new Downloader;
    this->ui->progressBar->setRange(0,100);
    this->ui->progressBar->reset();
    connect(downloader,&Downloader::downloadComplete,this,&MainWindow::ok);
    connect(downloader,&Downloader::downloadUpdate,[this](int progress) {
        this->ui->progressBar->setValue(progress);
        qDebug()<<progress;
    });
    ui->label->setText("Download Started!");
    downloader->start(ui->lineEdit->text(),".");
}
