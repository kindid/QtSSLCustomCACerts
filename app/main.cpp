#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>

// this exists only to trap problems. If you install this component in the QmlEngine
// you will have a useful place to poke about for SSL Errors.
//#include <KindidSecureNAMFactory.h>

int main(int argc, char *argv[])
{
    // Load the certificate
    QFile file(":/cert/dummycom_root_ca.pem");
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Could not load certificate!");
    }
    const QByteArray bytes = file.readAll();
    const QSslCertificate certificate(bytes);
    /// Add our own CA to the default SSL configuration
    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    auto certs = configuration.caCertificates();
    certs.append(certificate);
    configuration.setCaCertificates(certs);
    QSslConfiguration::setDefaultConfiguration(configuration);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    
//    engine.setNetworkAccessManagerFactory(new KindidSecureNAMFactory(certificate));
    
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    
    return app.exec();
}
