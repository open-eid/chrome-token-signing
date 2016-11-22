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

// inject page.js to the DOM of every page
// FIXME: maybe not ?
var s = document.createElement('script');
s.src = chrome.extension.getURL('page.js');

// remove script tag after script itself has loaded
s.onload = function() {this.parentNode.removeChild(this);};
(document.head || document.documentElement).appendChild(s);
