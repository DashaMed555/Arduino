const express = require('express');
const { networkInterfaces } = require('os');
const path = require('path');
 
const app = express();
const nets = networkInterfaces();
 
// Server port
const PORT = 3000;
 
app.get('/', (request, response) => response.send('Hello from www.mischianti.org!'));
 
let downloadCounter = 1;
const LAST_VERSION = 0.2;
app.get('/update', (request, response) => {
    const version = (request.header("x-esp8266-version"))?parseFloat(request.header("x-esp8266-version")):Infinity;
 
    if (version<LAST_VERSION){
        // If you need an update go here
        response.status(200).download(path.join(__dirname, 'firmware/httpUpdateNew.bin'), 'httpUpdateNew.bin', (err)=>{
            if (err) {
                console.error("Problem on download firmware: ", err)
            }else{
                downloadCounter++;
            }
        });
        console.log('Your file has been downloaded '+downloadCounter+' times!')
    }else{
        response.status(304).send('No update needed!')
    }
});
 
 
app.listen(PORT, () => {
    const results = {}; // Or just '{}', an empty object
 
    for (const name of Object.keys(nets)) {
        for (const net of nets[name]) {
            // Skip over non-IPv4 and internal (i.e. 127.0.0.1) addresses
            if (net.family === 'IPv4' && !net.internal) {
                if (!results[name]) {
                    results[name] = [];
                }
                results[name].push(net.address);
            }
        }
    }
 
    console.log('Listening on port '+PORT+'\n', results)
});
