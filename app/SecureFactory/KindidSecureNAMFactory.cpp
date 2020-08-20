#include "KindidSecureNAMFactory.h"

#include <KindidNAM.h>

#include <here.h>

KindidSecureNAMFactory::KindidSecureNAMFactory(const QSslCertificate & certificate)
    : certificate(certificate)
{ }

QNetworkAccessManager * KindidSecureNAMFactory::create(QObject *parent)
{
    here << "KindidSecureNAMFactory::create";
    return new KindidNAM(certificate, parent);
}
