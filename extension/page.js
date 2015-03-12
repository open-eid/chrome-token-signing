/* Chrome token signing extension
 *
 * This software is released under the GNU Library General Public License.
 * See LICENSE.LGPL
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */
// Promises
var _eid_promises = {};
// Turn the incoming message from extension
// into pending Promise resolving
window.addEventListener("message", function(event) {
    if(event.source !== window) return;
    if(event.data.src && (event.data.src === "background.js")) {
        console.log("Page received: ");
        console.log(event.data);
        // Get the promise
        if(event.data.nonce) {
            var p = _eid_promises[event.data.nonce];
            // resolve
            if(event.data.result === "ok") {
                if(event.data.signature !== undefined) {
                    p.resolve(event.data.signature);
                } else if(event.data.version !== undefined) {
                    p.resolve(event.data.version);
                } else if(event.data.cert !== undefined) {
                    p.resolve(event.data.cert);
                } else {
                    console.log("No idea how to handle message");
                    console.log(event.data);
                }
            } else {
                console.log("fail");
                // reject
                p.reject(new Error(event.data.result));
            }
            delete _eid_promises[event.data.nonce];
        } else {
            console.log("No nonce in event msg");
        }
    }
}, false);


function TokenSigning(lang) {
    function nonce() {
        var val = "";
        var hex = "abcdefghijklmnopqrstuvwxyz0123456789";
        for(var i = 0; i < 16; i++) val += hex.charAt(Math.floor(Math.random() * hex.length));
        return val;
    }
    if(!lang || lang === undefined) {
        lang = 'en';
    }

    function messagePromise(msg) {
        return new Promise(function(resolve, reject) {
            // amend with necessary metadata
            msg['nonce'] = nonce();
            msg['src'] = 'page.js';
            msg['lang'] = lang;
            // send message
            window.postMessage(msg, "*");
            // and store promise callbacks
            _eid_promises[msg.nonce] = {
                resolve: resolve,
                reject: reject
            };
        });
    }
    this.getCertificate = function() {
        console.log("getCertificate()");
        return messagePromise({
            type: 'CERT'
        });
    };
    this.sign = function(cert, hash) {
        console.log("sign()");
        return messagePromise({
            type: 'SIGN',
            cert: cert,
            hash: hash
        });
    };
    this.getVersion = function() {
        console.log("getVersion()");
        return messagePromise({
            type: 'VERSION'
        });
    };
}
