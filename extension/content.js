/*
 * Chrome token signing extension
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

var inuse = false;

// Forward the message from page.js to background.js
window.addEventListener("message", function(event) {
    // We only accept messages from ourselves
    if (event.source !== window)
        return;

    // and forward to extension
    if (event.data.src && (event.data.src === "page.js")) {
        event.data["origin"] = location.origin;
        chrome.runtime.sendMessage(event.data, function(response) {});

        // Only add unload handler if extension has been used
        if (!inuse) {
            // close the native component if page unloads
            window.addEventListener("beforeunload", function(event) {
                chrome.runtime.sendMessage({src: 'page.js', type: 'DONE'});
            }, false);
            inuse = true;
        }
    }
}, false);

// post messages from extension to page
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    window.postMessage(request, '*');
});

var pageScript = function() {
    // Promises
    var _eid_promises = {};
    // Turn the incoming message from extension
    // into pending Promise resolving
    window.addEventListener("message", function(event) {
        if (event.source !== window) return;
        if (event.data.src && (event.data.src === "background.js")) {
            console.log("Page received: ");
            console.log(event.data);
            // Get the promise
            if (event.data.nonce) {
                var p = _eid_promises[event.data.nonce];
                // resolve
                if (event.data.result === "ok") {
                    if (event.data.signature !== undefined) {
                        p.resolve({hex: event.data.signature});
                    } else if (event.data.version !== undefined) {
                        p.resolve(event.data.extension + "/" + event.data.version);
                    } else if (event.data.cert !== undefined) {
                        p.resolve({hex: event.data.cert});
                    } else {
                        console.log("No idea how to handle message");
                        console.log(event.data);
                    }
                } else {
                    // reject
                    p.reject(new Error(event.data.result));
                }
                delete _eid_promises[event.data.nonce];
            } else {
                console.log("No nonce in event msg");
            }
        }
    }, false);

    window.TokenSigning = function() {
        function nonce() {
            var val = "";
            var hex = "abcdefghijklmnopqrstuvwxyz0123456789";
            for (var i = 0; i < 16; i++) val += hex.charAt(Math.floor(Math.random() * hex.length));
            return val;
        }

        function messagePromise(msg) {
            return new Promise(function(resolve, reject) {
                // amend with necessary metadata
                msg["nonce"] = nonce();
                msg["src"] = "page.js";
                // send message
                window.postMessage(msg, "*");
                // and store promise callbacks
                _eid_promises[msg.nonce] = {
                    resolve: resolve,
                    reject: reject
                };
            });
        }
        this.getCertificate = function(options) {
            var msg = {type: "CERT", lang: options.lang, filter: options.filter};
            console.log("getCertificate()");
            return messagePromise(msg);
        };
        this.sign = function(cert, hash, options) {
            var msg = {type: "SIGN", cert: cert.hex, hash: hash.hex, hashtype: hash.type, lang: options.lang, info: options.info};
            console.log("sign()");
            return messagePromise(msg);
        };
        this.getVersion = function() {
            console.log("getVersion()");
            return messagePromise({
                type: "VERSION"
            });
        };
    };
};

/**
 * Check the page for an existing TokenSigning page script.
 * The script will be injected to the DOM of every page, which doesn't already have the script.
 * To circumvent Content Security Policy issues, the website can include the script on its own.
 *
 * Example:
 *   <script src="path-to/page.js" data-name="TokenSigning"></script>
 *
 * The page script can be found here:
 *   https://github.com/open-eid/chrome-token-signing/blob/master/extension/page.js
 */
if (!document.querySelector("script[data-name='TokenSigning']")) {
    var s = document.createElement("script");
    s.type = "text/javascript";
    s.dataset.name = "TokenSigning";
    s.innerHTML = "(" + pageScript + ")();";

    (document.head || document.documentElement).appendChild(s);
}
