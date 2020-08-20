#ifndef KINDIDSECURENAMFACTORY_H
#define KINDIDSECURENAMFACTORY_H

#include <QQmlNetworkAccessManagerFactory>
#include <QSslCertificate>

// this is a simple factory interface - we keep a copy of the
// certificate in here because we'll want to give that to all
// of the network access managers created
class KindidSecureNAMFactory : public QQmlNetworkAccessManagerFactory
{
public:
    explicit KindidSecureNAMFactory(const QSslCertificate & certificate);
    
    // only overide...
    virtual QNetworkAccessManager * create(QObject *parent);
    
signals:
    
private:
    QSslCertificate certificate;
    
};

#endif // KINDIDSECURENAMFACTORY_H
