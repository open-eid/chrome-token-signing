/* Chrome token signing extension
 *
 * This software is released under the GNU Library General Public License.
 * See LICENSE.LGPL
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */
var NO_NATIVE_URL = "https://pubkey.ee/sign/no_native_installed.html";
var HELLO_URL = "https://pubkey.ee/sign/hello.html";
var NATIVE_HOST = "ee.ria.esteid";
var K_SRC = "src";
var K_ORIGIN = "origin";
var K_NONCE = "nonce";
var K_RESULT = "result";
// Store the nonce/tabID pairs
var sessions = {};
// Stores the longrunning ports per origin
var ports = {};

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
// When extension is installed
chrome.runtime.onInstalled.addListener(function(details) {
	// Check if native is available or direct to help page.
	chrome.tabs.create({'url': HELLO_URL + "?" + details.reason});
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
	sessions[request[K_NONCE]] = sender.tab.id;
	send(request);
});
// Send the message back to the originating tab
function sendResponse(msg) {
	if(msg) {
		console.log("RECV: " + JSON.stringify(msg));
		// Add this
		msg[K_SRC] = "background.js";
		// Look up the nonce and send to the same tab
		var nonce = msg[K_NONCE];
		if (nonce) {
			chrome.tabs.sendMessage(sessions[nonce], msg, function(response) {});
		} else {
			console.log("No nonce in response!");
		}
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
	if(stateless) {
		chrome.runtime.sendNativeMessage(NATIVE_HOST, message, function(response) {
			// in case of error we have the originating message at hand for nonce
			if (!response) {
				console.log("ERROR: " + JSON.stringify(chrome.runtime.lastError));
				var resp = {};
				resp[K_SRC] = "background.js";
				resp[K_NONCE] = message[K_NONCE];
				// XXX: not really nice but seems the only way to detect other errors
				var EXPECTED_MESSAGE = "Specified native messaging host not found."
				if (chrome.runtime.lastError && chrome.runtime.lastError.message === EXPECTED_MESSAGE) {
					resp[K_RESULT] = "no_implementation";
				} else {
					resp[K_RESULT] = "technical_error";
				}
				// TODO: Chrome 41+ allows to send to a specific frame.
				chrome.tabs.sendMessage(sessions[resp["nonce"]], resp);
			} else {
				sendResponse(response);
			}
		});
	} else {
		if(!ports[message[K_ORIGIN]]) {
			console.log("Connecting to native messaging host " + NATIVE_HOST);
			var port = chrome.runtime.connectNative(NATIVE_HOST);
			port.onMessage.addListener(sendResponse);
			port.onDisconnect.addListener(onDisconnected);
			ports[message[K_ORIGIN]] = port;
		}
		ports[message[K_ORIGIN]].postMessage(message);
	}
}
