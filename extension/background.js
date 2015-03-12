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
var DEVELOPER_URL = "https://github.com/open-eid/chrome-token-signing/wiki/DeveloperTips";

var NATIVE_HOST = "ee.ria.esteid";

var K_SRC = "src";
var K_ORIGIN = "origin";
var K_NONCE = "nonce";
var K_RESULT = "result";
var K_TAB = "tab";

// Stores the longrunning ports per tab
// Used to route all request from a tab to the same host instance
var ports = {};

// Probed to false if host component is OK.
var missing = true;

console.log("Background page activated");

// XXX: probe test, because connectNative() does not allow to check the presence
// of native component for some reason
_testNativeComponent().then(function(result) {
	if (result === "ok")
		missing = false;
});

// Force kill of native process
// Becasue Port.disconnect() does not work
function _killPort(tab) {
	if (tab in ports) {
		console.log("KILL " + tab);
		// Force killing with an empty message
		ports[tab].postMessage({});
	}
}

// Check if native implementation is OK resolves with "ok", "missing" or "forbidden"
function _testNativeComponent() {
	return new Promise(function(resolve, reject) {
		chrome.runtime.sendNativeMessage(NATIVE_HOST, {}, function(response) {
			if (!response) {
				console.log("TEST: ERROR " + JSON.stringify(chrome.runtime.lastError));
				// Try to be smart and do some string matching
				var permissions = "Access to the specified native messaging host is forbidden.";
				var missing = "Specified native messaging host not found.";
				if (chrome.runtime.lastError.message === permissions) {
					resolve("forbidden")
				} else if (chrome.runtime.lastError.message === missing) {
					resolve("missing");
				} else {
					resolve("missing");
				}
			} else {
				console.log("TEST: " + JSON.stringify(response));
				if (response["result"] === "invalid_argument") {
					resolve("ok");
				} else {
					resolve("missing"); // TODO: something better here
				}
			}
		});
	});
}


// When extension is installed, check for native component or direct to helping page
chrome.runtime.onInstalled.addListener(function(details) {
	if (details.reason === "install" || details.reason === "update") {
		_testNativeComponent().then(function(result) {
				var url = null;
				if (result === "ok" && details.reason === "install") {
					url = HELLO_URL;
				} else if (result === "forbidden") {
					url = DEVELOPER_URL;
				} else if (result === "missing"){
					url = NO_NATIVE_URL;
				}
				if (url) {
					chrome.tabs.create({'url': url + "?" + details.reason});
				}
		});
	}
});

// When message is received from page send it to native
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
	if(sender.id !== chrome.runtime.id) {
		console.log('WARNING: Ignoring message not from our extension');
		// Not our extension, do nothing
		return;
	}
	if (sender.tab) {
		// Check if page is DONE and close the native component without doing anything else
		if (request["type"] === "DONE") {
			console.log("DONE " + sender.tab.id);
			if (sender.tab.id in ports) {
				// FIXME: would want to use Port.disconnect() here
				_killPort(sender.tab.id);
			} 
		} else {
			request[K_TAB] = sender.tab.id;
			if (missing)
				return _fail_with(request, "no_implementation");
			// TODO: Check if the URL is in allowed list or not
			// Either way forward to native currently
			_forward(request);
		}
	}
});

// Send the message back to the originating tab
function _reply(tab, msg) {
	msg[K_SRC] = "background.js";
	chrome.tabs.sendMessage(tab, msg);
}

// Fail an incoming message if the underlying implementation is not
// present
function _fail_with(msg, result) {
	var resp = {};
	resp[K_NONCE] = msg[K_NONCE];
	resp[K_RESULT] = result;
	_reply(msg[K_TAB], resp);
}

// Forward a message to the native component
function _forward(message) {
	var tabid = message[K_TAB];
	console.log("SEND " + tabid + ": " + JSON.stringify(message));
	// Open a port if necessary
	if(!ports[tabid]) {
		// For some reason there does not seem to be a way to detect missing components from longrunning ports
		// So we probe before opening a new port.
		console.log("OPEN " + tabid + ": " + NATIVE_HOST);
		// create a new port
		var port = chrome.runtime.connectNative(NATIVE_HOST);
		// XXX: does not indicate anything for some reason.
		if (!port) {
			console.log("OPEN ERROR: " + JSON.stringify(chrome.runtime.lastError));
		}
		port.onMessage.addListener(function(response) {
			if (response) {
				console.log("RECV "+tabid+": " + JSON.stringify(response));
				_reply(tabid, response);
			} else {
				console.log("ERROR "+tabid+": " + JSON.stringify(chrome.runtime.lastError));
				_fail_with(message, "technical_error");
			}
		});
		port.onDisconnect.addListener(function() {
			console.log("QUIT " + tabid);
			delete ports[tabid];
			// TODO: reject all pending promises for tab, if any
		});
		ports[tabid] = port;
		ports[tabid].postMessage(message);
	} else {
		// Port already open
		ports[tabid].postMessage(message);
	}
}
