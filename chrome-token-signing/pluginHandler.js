/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

var EstEidCallbackHolder = {
    successCallback: null,
    failureCallback: null
};

window.addEventListener("message", function(event) {
    if (event.source !== window)
        return;
    
    if (event.data.source && (event.data.source === "FROM_WORKER")) {
        console.log("Page received: ");
        console.log(event);
        if (event.data.returnCode === undefined) {
            console.log("success");
            var callbackData = event.data;
            if (event.data.signature !== undefined) {
              callbackData = event.data.signature;
            }
            if (event.data.version !== undefined) {
              callbackData = event.data.version;
            }
            EstEidCallbackHolder.successCallback(callbackData);
        } else {
            console.log("failure");
            EstEidCallbackHolder.failureCallback(new IdCardException(event.data.returnCode, event.data.message));
        }
        //EstEidCallbackHolder.successCallback = null;
        //EstEidCallbackHolder.failureCallback = null;
    }
}, false);

function getType() {
    return 'ASYNC';
}

function loadSigningPlugin(lang, pluginToLoad) {
    return;
}

function checkIfPluginIsLoaded(pluginName, lang) {
  return true;
}

function isPluginSupported(pluginName) {
  return true;
}

function IdCardPluginHandler(lang) {
    if (!lang || lang === undefined) {
        lang = 'eng';
    }

    this.getCertificate = function(successCallback, failureCallback) {
        window.postMessage({source: "FROM_PAGE", type: "CERT", lang: lang}, "*");
        EstEidCallbackHolder.successCallback = successCallback;
        EstEidCallbackHolder.failureCallback = failureCallback;
        console.log("Cert");
    };

    this.sign = function(id, hash, successCallback, failureCallback) {
        window.postMessage({source: "FROM_PAGE", type: "SIGN", lang: lang, hash: hash, id: id}, "*");
        EstEidCallbackHolder.successCallback = successCallback;
        EstEidCallbackHolder.failureCallback = failureCallback;
        console.log("Allkirjastan");
    };

    this.getVersion = function(successCallback, failureCallback) {
        window.postMessage({source: "FROM_PAGE", type: "VERSION", lang: lang}, "*");
        EstEidCallbackHolder.successCallback = successCallback;
        EstEidCallbackHolder.failureCallback = failureCallback;
        console.log("Version");
    };

}