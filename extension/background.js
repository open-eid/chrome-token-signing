/* Chrome token signing extension
 *
 * This software is released under the GNU Library General Public License.
 * See LICENSE.LGPL
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */
// Store the nonce/tabID pairs
var sessions = {};
var port = null;
console.log("Background page activated");
// When extension is loaded
chrome.runtime.onSuspend.addListener(function() {
    console.log("Suspending");
    // Disconnect the port if necessary
    if(port) {
        port.disconnect();
    }
});
// When extension is loaded
chrome.runtime.onConnect.addListener(function(port) {
    console.log("Connection from " + port.name);
});
// When message is received from page
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    console.log("Marshal from url " + sender.url + " and extension id  " + sender.id);
    // Check that the sender is extension
    if(sender.id !== chrome.runtime.id) {
        console.log('Ignoring message not from our extension');
        // Not our extension, do nothing
        return;
    }
    // TODO: Check if the URL is in allowed list or not
    // Either way forward to native currently
    sessions[request['nonce']] = sender.tab.id;
    send(request);
});
// Send the message back to the originating tab
function sendResponse(msg) {
    if(msg) {
        console.log("RECV: " + JSON.stringify(msg));
        // Add this
        msg["src"] = "background.js";
        // Look up the nonce and send to the same tab
        var nonce = msg['nonce'];
        chrome.tabs.sendMessage(sessions[nonce], msg, function(response) {});
    } else {
        console.log("ERROR: " + JSON.stringify(chrome.runtime.lastError));
    }
}

function onDisconnected() {
    console.log("Connection closed: " + chrome.runtime.lastError.message);
}

function send(message) {
    var stateless = true;
    console.log("SEND: " + JSON.stringify(message));
    console.log("SEND is " + stateless ? "stateless" : "longrunning");
    var hostName = "ee.ria.esteid";
    if(stateless) {
        chrome.runtime.sendNativeMessage(hostName, message, sendResponse);
    } else {
        if(!port) {
            console.log("Connecting to native messaging host " + hostName);
            port = chrome.runtime.connectNative(hostName);
            port.onMessage.addListener(sendResponse);
            port.onDisconnect.addListener(onDisconnected);
        }
        port.postMessage(message);
    }
}
