#include "KindidNAM.h"
#include <here.h>

KindidNAM::KindidNAM(const QSslCertificate & certificate, QObject *parent)
    : QNetworkAccessManager(parent)
    , certificate(certificate)
{
}

QNetworkReply *	KindidNAM::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData)
{
    here << "createRequest" << op << originalReq.url() << outgoingData;

    auto nr = QNetworkAccessManager::createRequest(op, originalReq, outgoingData);

    connect(nr, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSSLErrors(const QList<QSslError> &)));

    return nr;
}

void KindidNAM::onSSLErrors(const QList<QSslError> &errors)
{
    auto nr = qobject_cast<QNetworkReply *>(sender());

    here << nr->url().host();

    foreach(auto e, errors) {
        here << "onSSLErrors " << e.errorString() << e.certificate() << int(e.error()) << e.certificate().expiryDate();
        if (QSslError::CertificateUntrusted) {
            if (certificate.expiryDate() < QDateTime::currentDateTime()) {
                qFatal("Certificate out of date");
            }
            // YOU SHOULD NOT DO THIS! THIS COMPONENT ONLY EXISTS FOR DIAGNOSTICS!!!
            nr->ignoreSslErrors();
        }
    }
}
