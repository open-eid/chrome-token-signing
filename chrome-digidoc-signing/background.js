/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    new EstEID().sign(request);
});

function sendResponse(messageFromHost) {
    chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
        messageFromHost["source"] = "FROM_WORKER";
        console.log(messageFromHost);
        chrome.tabs.sendMessage(tabs[0].id, messageFromHost, function(response) {
        });
    });
}

var EstEID = function() {
    var port = null;

    function sendNativeMessage(message) {
        port.postMessage(message);
    }

    function onNativeMessage(message) {
        console.log("Message from host: ");
        console.log(message);
        sendResponse(message);
    }

    function onDisconnected() {
        console.log("Failed to connect: " + chrome.runtime.lastError.message);
        port = null;
    }

    function connect() {
        var hostName = "ee.ria.esteid";
        console.log("Connecting to native messaging host " + hostName);
        port = chrome.runtime.connectNative(hostName);
        port.onMessage.addListener(onNativeMessage);
        port.onDisconnect.addListener(onDisconnected);
    }

    this.sign = function(message) {
        connect();
        console.log(message);
        sendNativeMessage(message);
    };
};
