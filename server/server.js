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
