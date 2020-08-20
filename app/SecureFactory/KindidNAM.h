#ifndef KINDIDNAM_H
#define KINDIDNAM_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslCertificate>

class KindidNAM : public QNetworkAccessManager
{
    Q_OBJECT
public:
    KindidNAM(const QSslCertificate & certificate, QObject *parent = nullptr);

    // overide this function in order to customise/delegate/derive the QNetworkReply
    // when our QNR is created we hook into the SSLErrors signal handler and invoke
    // ignoreSSLErrors() at the appropriate time
    virtual QNetworkReply *	createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData = nullptr);

protected slots:
    // handle the SSLErrors here
    void onSSLErrors(const QList<QSslError> &errors);

private:
    QSslCertificate certificate;
};

#endif // KINDIDNAM_H
