#include "downloader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFileInfo>
#include <QUrl>
#include <QDir>

Download::Download(qulonglong index, QObject *parent) : QObject(parent)
{
    this->index = index;
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
    this->range = QString("Bytes=%1-%2").arg(startPoint).arg(endPoint);
    qheader.setRawHeader("Range", this->range.toLatin1());  //range.toLatin1()转换range为ASCII的QByteArray

    QNetworkRequest request(qheader);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36 Edg/88.0.705.56");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    this->reply = this->manager.get(request);

    connect(this->reply,&QNetworkReply::finished,this,&Download::finishedPart);
    connect(this->reply,&QNetworkReply::readyRead,this,&Download::readReady);
}

void Download::finishedPart() {
    this->file->flush();
    this->reply->deleteLater();
    this->reply = 0;
    this->file = 0;
    emit this->downloadComplete();
}

void Download::readReady() {
    if (!this->file) {
        return;
    }
    QByteArray buffer = this->reply->readAll();
    this->file->seek(this->startPoint + this->bytesDownload);
    this->file->write(buffer);
    this->bytesDownload += buffer.size();
    emit this->downloadUpdate(buffer.size());
}

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    this->numberComplet = 0;
    this->fileSize = 0;
    this->bytesDownload = 0;
}

qulonglong Downloader::getFileSize(QUrl url) {
    QNetworkAccessManager manager;
    QEventLoop loop;
    //获取请求头
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36 Edg/88.0.705.56");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *reply = manager.head(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit, Qt::DirectConnection);
    loop.exec();
    //获取文件大小
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    reply->deleteLater();
    return var.toULongLong();
}

void Downloader::start(const QString &url, const QString &dir) {
    this->dir = QDir(dir);
    this->url = QUrl(url);
    this->fileSize = getFileSize(this->url);
    QFileInfo fileInfo(this->url.path());
    this->fileName = fileInfo.fileName();
    if (this->fileName.isEmpty()) {
        this->fileName = "index.html";
    }
    this->dir.makeAbsolute();
    this->file = new QFile(this->dir.filePath(this->fileName));
    this->file->setFileName(fileName);
    this->file->open(QIODevice::WriteOnly);
    Download *temp;

    for (int i = 0; i < this->numberThreads; i++) {
        qulonglong start = fileSize * i / this->numberThreads;
        qulonglong end = fileSize * (i+1) / this->numberThreads;
        temp = new Download(i+1);
        connect(temp,&Download::downloadComplete,this,&Downloader::subPartComplete);
        connect(temp,&Download::downloadComplete,temp,&Download::deleteLater);
        connect(temp,&Download::downloadUpdate,[this](qulonglong bytes){
            this->bytesDownload += bytes;
            qDebug()<<"bytesDownload "<<this->bytesDownload;
            emit downloadUpdate(this->bytesDownload * 100 / this->fileSize);
        });
        temp->start(this->url,this->file,start,end);
    }
}

void Downloader::subPartComplete() {
    this->numberComplet++;
    if (this->numberComplet == this->numberThreads) {
        this->file->close();
        emit this->downloadComplete();
    }
}
