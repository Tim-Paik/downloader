#include "downloader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFileInfo>
#include <QUrl>

Download::Download(QObject *parent) : QObject(parent)
{
    this->bytesDownload = 0;
    this->startPoint = 0;
    this->endPoint = 0;
    this->file = NULL;
}

void Download::start(const QUrl &url, QFile *file,
                     qulonglong startPoint, qulonglong endPoint) {
    if (NULL == file) {
        return;
    }
    this->bytesDownload = 0;
    this->startPoint = startPoint;
    this->endPoint = endPoint;
    this->file = file;

    QNetworkRequest qheader;
    qheader.setUrl(url);
    //this->range.sprintf("Bytes=%lld-%lld",startPoint,endPoint);
    this->range = QString("Bytes=%1-%2").arg(startPoint).arg(endPoint);
    qheader.setRawHeader("Range", this->range.toLatin1());  //range.toLatin1()转换range为ASCII的QByteArray

    QNetworkRequest request(qheader);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    this->reply = this->manager.get(request);

    connect(this->reply,SIGNAL(finished()),this,SLOT(finishedPart()));
    connect(this->reply,SIGNAL(readyRead()),this,SLOT(readReady()));
}

void Download::finishedPart() {
    this->file->flush();
    //this->reply->deleteLater();
    //this->reply = 0;
    //this->file = 0;
    qDebug()<<"finishedPart";
    emit this->downloadComplete();
    qDebug()<<"downloadComplete emited";
}

void Download::readReady() {
    if (!this->file) {
        return;
    }
    QByteArray buffer = this->reply->readAll();
    this->file->seek(this->startPoint + this->bytesDownload);
    this->file->write(buffer);
    this->bytesDownload += buffer.size();
}

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    this->numberComplet = 0;
    this->fileSize = 0;
    this->file = new QFile;
}

qulonglong Downloader::getFileSize(QUrl url) {
    QNetworkAccessManager manager;
    QEventLoop loop;
    //获取请求头
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *reply = manager.head(request);
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
    loop.exec();
    //获取文件大小
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    reply->deleteLater();
    qDebug()<<var.toULongLong();
    return var.toULongLong();
}

void Downloader::start(const QString &url) {
    this->url = QUrl(url);
    this->fileSize = getFileSize(this->url);
    QFileInfo fileInfo(this->url.path());
    this->fileName = fileInfo.fileName();
    if (this->fileName.isEmpty()) {
        this->fileName = "index.html";
    }
    this->file->setFileName(fileName);
    this->file->open(QIODevice::WriteOnly);
    Download *temp;

    for (int i = 0; i < this->numberThreads; i++) {
        qulonglong start = fileSize * i / this->numberThreads;
        qulonglong end = fileSize * (i+1) / this->numberThreads;
        temp = new Download();
        connect(temp,SIGNAL(downloadComplete()),this,SLOT(subPartComplete()));
        connect(temp,SIGNAL(downloadComplete()),temp,SLOT(deleteLater()));
        temp->start(this->url,this->file,start,end);
        qDebug()<<start<<end;
    }
}

void Downloader::subPartComplete() {
    qDebug()<<"subPartComplete";
    this->numberComplet++;
    qDebug()<<this->numberComplet;
    if (this->numberComplet == this->numberThreads) {
        qDebug()<<"Complet";
        this->file->close();
        emit this->downloadComplete();
    }
}
