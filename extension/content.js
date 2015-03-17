/* Chrome token signing extension
 *
 * This software is released under the GNU Library General Public License.
 * See LICENSE.LGPL
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
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

// inject page.js to the DOM of every page
// FIXME: maybe not ?
var s = document.createElement('script');
s.src = chrome.extension.getURL('page.js');

// remove script tag after script itself has loaded
s.onload = function() {this.parentNode.removeChild(this);};
(document.head || document.documentElement).appendChild(s);
