/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

window.addEventListener("message", function(event) {
    // We only accept messages from ourselves
    if (event.source !== window)
        return;

    if (event.data.source && (event.data.source === "FROM_PAGE")) {
        console.log("Worker received: ");
        console.log(event);
        event.data["protocol"] = location.protocol;
        chrome.runtime.sendMessage(event.data, function(response) {
            
        });
    }
}, false);

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    window.postMessage(request, '*');
});

var s = document.createElement('script');
s.src = chrome.extension.getURL('pluginHandler.js');
s.onload = function() {this.parentNode.removeChild(this);};
(document.head || document.documentElement).appendChild(s);
