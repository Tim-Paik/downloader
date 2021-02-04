#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkAccessManager>

//单线程下载
class Download : public QObject
{
    Q_OBJECT
public:
    Download(QObject *parent = nullptr);
    void start(const QUrl &url, QFile *file,
               qulonglong startPoint=0, qulonglong endPoint=-1);

public slots:
    void finishedPart();
    void readReady();

signals:
    void downloadComplete();

private:
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QFile *file;
    qulonglong bytesDownload;
    qulonglong startPoint;
    qulonglong endPoint;
    QString range;
};

//多线程并发下载
class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);
    void setNumberThreads(ushort numberThreads) { this->numberThreads = numberThreads; }
    quint16 getNumberThreads() { return this->numberThreads; }
    qulonglong getFileSize(QUrl url);
    void start(const QString &url);

signals:
    void downloadComplete();

private slots:
    void subPartComplete();

private:
    quint16 numberThreads = 32; //最大限制在65535
    qint16 numberComplet = 0;
    qulonglong fileSize;
    QUrl url;
    QFile *file;
    QString fileName;
};

#endif // DOWNLOADER_H
