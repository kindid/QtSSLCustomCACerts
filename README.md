# Generating Working Self-Signed Certificates for iOS/Android etc for a Qt/QML Project

## Contents of Repo

- A basic web service that is protected by SSL written in Node.JS using Express and the HTTPS module.
- A simple QML application that accesses data from the server (above)

## Goals

1. To create a Qt/QML application that can securely access our own data sources
1. Use certificate siging so that our apps and services can trust a broad range of our own services hosted on different servers etc
1. Be in control of the chain of trust
1. Save some money due to not having to pay for certificate signing/renewal

## Concepts

We will establish a secure connect 
Something about *why* we do this. Secure channel, validation of end points (srver, apps etc)

Secure communications and the trusting of sources are done using a collection of certificates. We will use our own root certificate to validate our own connections (see Root CA)

### Android Specifics

For Android we will be using the Android build of the OpenSSL libraries from KDAB (build them yourself if you're really paranoid). AFAICT this is just an implementation of SSL and I'm not sure Android makes any further hardening restrictions on connections. This is available, pre-built at https://github.com/KDAB/android_openssl.git

If you don't trust those binaries then you can build them yourself. Instructions here https://doc.qt.io/qt-5/android-openssl-support.html

Simply run `git clone https://github.com/KDAB/android_openssl.git` in the root folder of this project.

### macOS/iOS specifics

For iOS/macOS the underlying mechanism is SecureTransport. This has additional hardening measures that we need to work with. Example - We can't use "CN" for domain validation and instead have to use "SAN" (more later). This, I believe, means we need a V3 x509 certificate instead of the default OpenSSL generate V1 certificate (again, more later). Mac/iOS also require AKI and EKU (I believe).

# Generating Certificates

## Root CA

A Root Certificate is used to sign other certificates. As with all certificates it comes in 3 parts. 

1. A key which is just a big blob of unpredictable random bits
1. A password used to secure the key - this is known only to "us" and needn't be store on the computer much like the PIN number for a bank card or an unlock code on a phone
1 . The certificate generated with that key - This requires the password and key (above). A configuration file is used as we will need to generate a V3 x509 certificate which has extensions over a V1 key. This is even more important when generating client keys

The Root CA is our own certificate that we will use to sign our other certificates. Our application only needs a copy of the our root certificate not all of the certificates that we signed with it.

If you're on a Mac you can export certificates from Key Chain Access and have a look at them - any x509 can be dumped using this command (this is staggering useful)

`opensll x509 -in some_cert_file.pem -text`

### Real World/Web Browsers

In the "real" world e.g. for web browser usage, we would not be able to use self signed certificates without getting our root certificate installed on every single device on the planet. Clearly your computer does not come with a certificate for every domain and IP address on the Internet (it would fill up first!) so Root CAs use their own (trusted) certificates to sign other certificates and only those Root Certificates need to be installed on your device. Again, Mac users can easily examine these with Key Chain Access.

Thankfully this is not our use case.

## Use of a Root Certificate

We use our *own* root certificate that ships with our application - we are not concerned. This allows us to trust any certifcate that is signed with our own Root Certificate (making us a Root CA). I use this feature to test locally and remotely. Certificates are bound to IP addresses of domain names (more on this later) so as I change servers and/or certificates I can still trust any of those certificates as long as they are signed with my own root certificate. You should now see why you must keep the key and password secure!

So we need to bundle our root certificate with our application so we can check 

## Creating the Root Certificate

Here is how to create a Root Certificate for our dummy organisation

Firstly create a key - *NOTE* I would **not** recommand placing the key password on the command line as it will be visible via `ps` or `ps -e` but it's simply here to demonstrate a point.

`openssl genrsa -des3 -passout pass:pingpongballeyes -out dummycom_root_ca.key 2048`

Secondly generate a root signing certificate using the key generated above (again, password on CLI is not a good idea)

`openssl req -x509 -new -nodes -key dummycom_root_ca.key -config dummycom_root_ca.cnf -extensions v3_req_ca -passin pass:pingpongballeyes -sha256 -days 18263 -out dummycom_root_ca.pem`

### Configuration of Root CA

In the command line given the file `dummycom_root_ca.cnf` is referenced. This contains the V3 extensions we need amongst other things.

```
[ req ]
distinguished_name = req_distinguished_name
req_extensions = v3_req_ca
prompt = no

[ v3_req_ca ]
basicConstraints = critical,CA:TRUE
keyUsage = critical,cRLSign,keyCertSign
subjectKeyIdentifier=hash
extendedKeyUsage = serverAuth,clientAuth
authorityKeyIdentifier=keyid:always,issuer:always

[ req_distinguished_name ]
C = BB
ST = COUNTYSHIRE
L = TOWNSVILLE
O = dummycom
OU = net
CN = dummy.com
```

The really, really important parts of this configuration file are `basicConstraints` and `extendedKeyUsage`. If we don't get this right then Secure Transport will *reject* your RC.

### Expiration

This certificate is valid for a *very* long time (just over 50 years) and, for me, that's how I want it. You should consider carefully what you want.

### Verify

That's it - you now have your own Root Certificate. Pat yourself on the back and run this command to see what's in the cert.

`openssl x509 -in dummycom_root_ca.pem -text`

## Installing the Root Certificate into a Qt Application

This is done via C++ code and is best done in, or a function called from, `main` when the application starts up. I store the certificate in PEM format and keep it in the Qt resources part of the app bundle.

```
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
```

That's it - you may want a more error checking or to be a little less fatalistic but that it gets the root certificate installed.

## Server Certificates

Now we generate a certificate for each server that our app will be communicating with. Personally I generate one for local testing and another for remote testing. Why? Because I *could* use Subject Alternate Name to specify the certificate works for both `IP:127.0.0.1` and `DNS:dummy.com` but I would leave my application vunerable as anyone would be able to run a server on local host and intercept my communications. In some ways they can already do this by changing the `/etc/hosts` file (and that's bad enough).

### Hacking `/etc/hosts` for Testing

Whilst we could use IP addresses I thought it would be fun to show this little trick.

Simply edit you `/etc/hosts` file like this...

`sudo nano /etc/hosts`

Then add the line

`127.0.0.1 fake.com`

The domain `fake.com` now redirects to local host - Fun eh? We'll use this feature in our server certificate setup.

If you do **NOT** want to do this then change the `subjectAltName` field to read `IP:127.0.0.1`

### Key Generation

Firstly generate a random, password protected, key. I create one of these per server. I guess you don't need to do that but I do.

`openssl genrsa -des3 -passout pass:sixslimyslugs -out fake.com.key 2048`

### Certificate Signing Request

Generate a `Certificate Signing Request`.  This will be used int he next step 

`openssl req -new -sha256 -key fake.com.key -passin pass:sixslimyslugs -config fake.com.cnf -out fake.com.csr`

We use a single configuration file in 2 steps here but the second uses a different extension (a block in the config file prefixed with [ $text ]). This is because we can't add the AKI (Authority Key Index) when we generate the signing request because the AKI is a back reference/hash to the Root Certificate we will use to sign.

Here is the configuration file for our server. Note the differences such as `CA:FALSE`, `subjectAlternateName` and the different key usages and extended key usages (mandatory on iOS).

```
[ req ]
distinguished_name = req_distinguished_name
req_extensions = v3_req
prompt = no

[ v3_req ]
basicConstraints = critical,CA:FALSE
keyUsage = critical,digitalSignature,keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = DNS:fake
subjectKeyIdentifier = hash

[ v3_aki_ext ]
authorityKeyIdentifier = keyid:issuer
basicConstraints = critical,CA:FALSE
keyUsage = critical,digitalSignature,keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = DNS:fake.com
subjectKeyIdentifier = hash

[ req_distinguished_name ]
C = BB
ST = COUNTYSHIRE
L = TOWNSVILLE
O = dummycom
OU = net
CN = dummy.com
```

### Sign the Server Certificate with out Root CA

Now we can sign our server certificate. We do this using our own root certificate, root certificate key and root certificate key password…

Notice the additional `-extensions v3_aki_ext`. This uses the additional section of the configuration file that tells OpenSSL to set the Authority Key Index in the generated, signed certificate. You can now see (above) that we could not have done this when generating the CSR as we did not have the RC to hand.

`openssl x509 -req -in fake.com.csr -CA dummycom_root_ca.pem -CAkey dummycom_root_ca.key -CAcreateserial -extfile fake.com.cnf -passin pass:pingpongballeyes -extensions v3_aki_ext -out fake.com.crt -days 825 -sha256`

### Expiration & Secure Transport

Secure Transport will **not** allow certificates to live for more than 825 days (about 2.25 years) so you *will* have to update your server every once in a while. In fact it is quite common for server certificates to require updating every year.

Note that our root certificate is very long lived so the app itself is good, in this case, for 50 years.

### Subject Alternate Name (SAN) vs Common Name (CN)

Use of SAN is now mandatory in Secure Transport. It is much more flexible than using the `CN` and lets you specify many different domains in a single certificate. I do not do this as my requirements are very simple.

If you wish to make a certificate for an IP address then just change the `SAN` to `subjectAltName = IP:127.0.0.1`

This is precisely what I do - my app does not use DNS at all but connects directly via IP address as this saves me a lot of latency and network traffic.

To illustrate a more complicated `SAN`  here's an extract from a Google certificate (edited) as extracted using `openssl x509 -in cert.pem -text`

`X509v3 Subject Alternative Name: 
DNS:*.google.com, DNS:*.android.com, DNS:*.appengine.google.com, DNS:*.bdn.dev, DNS:*.cloud.google.com, DNS:*.crowdsource.google.com, DNS:*.g.co, DNS:*.gcp.gvt2.com, DNS:*.gcpcdn.gvt1.com, DNS:*.ggpht.cn, DNS:*.gkecnapps.cn, DNS:*.google-analytics.com, DNS:*.google.ca...`

In "the old days" the CN was used to match the requested domain to the certificate and vice versa. This is *not* the case anymore at least under Secure Transport

### IP Address instead of DNS

As shown above you can use IP addresses in the SAN field instead of a domain name. I have provided example configuration files in the `ssl` folder.

# Setting up the Server

We will use Node.JS with the Express module to test our server.

On the server we need to place our server (fake) key, certificate and the key password - there are probably better ways to do this than demonstrated (example - a config file or config JS script) but this illustrates the point

In the subfolder called `cert` are out crt (pem) file and key for the `fake.com` certificate we signed with our dummy root certificate. Remember, our app has the root certificate inside and adds this to its SSL configuration and uses this to validate the `fake.com` certificate

```
// fs required to load certificates
const fs = require('fs')
// express used as server middleware
const express = require('express');
// https gives a crypto plugin for express
const https = require('https');
// port to listen on (make this ≥ 1024 unless you want to run as SU)
const port_ssl = 6666;
// load the certificates
const options = {
    key: fs.readFileSync('cert/fake.com.key'),
    cert: fs.readFileSync('cert/fake.com.crt'),
    passphrase: 'sixslimyslugs' }
const app = express();
        app.get('/', (req, res) => {
            res.json({status:'alive', alive: true})
        });
// start the server 
https.createServer(options, app).listen(port_ssl);
```

Now all you need to do is fire up the server e.g.

`node server.js`

## Testing with CURL

First lets try to access the server directly

`curl https://fake.com:6666`

…and it fails - as it should. The certificate the server offered up (fake.crt) can not be trusted as the root certificate does not exist. So let's offer up the root certificate (go into the )

`curl --cacert dummmycom_root_ca.pem https://fake.com:6666/`

**MAGIC**

We now get back our JSON data from the server.

## Prologue

I hope you enjoyed this. If you find any problems or improvements please let me know - I will be every so grateful!

I also hope this is useful to you. It took me around 4 days to fully(!) understand and implement all this so I hope this knowledge dispersal helps you.


## Last Notes/127.0.0.1

Here are some instructions (run this in the `ssl` folder) to create a server certificate for `127.0.0.1`

```
$ openssl genrsa -des3 -passout pass:sixslimyslugs -out 127.0.0.1.key 2048
$ openssl req -new -sha256 -key 127.0.0.1.key -passin pass:sixslimyslugs -config 127.0.0.1.cnf -out 127.0.0.1.csr
$ openssl x509 -req -in 127.0.0.1.csr -CA dummycom_root_ca.pem -CAkey dummycom_root_ca.key -CAcreateserial -extfile 127.0.0.1.cnf -passin pass:pingpongballeyes -extensions v3_aki_ext -out 127.0.0.1.crt -days 825 -sha256
```
