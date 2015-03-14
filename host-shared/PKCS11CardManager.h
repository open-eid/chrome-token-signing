/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

#pragma once

#include "pkcs11.h"
#include "Logger.h"
#include "DateUtils.h"
#include "PinString.h"

#include <openssl/x509.h>
#include <dlfcn.h>

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA384_LENGTH 48
#define BINARY_SHA512_LENGTH 64

#ifndef PKCS11_MODULE
#ifdef _WIN32
#define PKCS11_MODULE "opensc-pkcs11.dll"
#elif defined(__APPLE__)
#define PKCS11_MODULE "/Library/EstonianIDCard/lib/esteid-pkcs11.so"
#else
#define PKCS11_MODULE "opensc-pkcs11.so"
#endif
#endif

class UserCanceledError : public std::runtime_error {
    public:
     UserCanceledError() : std::runtime_error("User canceled"){}
};

class AuthenticationError : public std::runtime_error {
public:
    AuthenticationError() : std::runtime_error("Authentication error"){}
};

class AuthenticationBadInput : public std::runtime_error {
public:
    AuthenticationBadInput() : std::runtime_error("Authentication Bad Input"){}
};

class PKCS11CardManager {
private:
    void *library = nullptr;
    CK_FUNCTION_LIST_PTR fl = nullptr;
    CK_TOKEN_INFO tokenInfo;
    CK_SESSION_HANDLE session = 0;
    X509 *cert = nullptr;
    std::vector<unsigned char> signCert;

    std::vector<CK_OBJECT_HANDLE> findObject(CK_OBJECT_CLASS objectClass, CK_ULONG max = 1) const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_ATTRIBUTE searchAttribute = {CKA_CLASS, &objectClass, sizeof(objectClass)};
        checkError("C_FindObjectsInit", fl->C_FindObjectsInit(session, &searchAttribute, 1));
        CK_ULONG objectCount = max;
        std::vector<CK_OBJECT_HANDLE> objectHandle(objectCount);
        checkError("C_FindObjects", fl->C_FindObjects(session, &objectHandle[0], objectHandle.size(), &objectCount));
        checkError("C_FindObjectsFinal", fl->C_FindObjectsFinal(session));
        objectHandle.resize(objectCount);
        return objectHandle;
    }

    std::string getFromX509Name(const std::string &subjectField) const {
        if (!cert) {
            throw std::runtime_error("Could not parse cert");
        }
        X509_NAME *name = X509_get_subject_name(cert);
        std::string X509Value(1024, 0);
        int length = X509_NAME_get_text_by_NID(name, OBJ_txt2nid(subjectField.c_str()), &X509Value[0], int(X509Value.size()));
        X509Value.resize(std::max(0, length));
        _log("%s length is %i, %p", subjectField.c_str(), length, X509Value.c_str());
        return X509Value;
    }

    void checkError(const std::string &methodName, CK_RV methodResult) const {
        if (methodResult != CKR_OK) {
            _log("%s return value %i", methodName.c_str(), methodResult);
            throw std::runtime_error("PKCS11 method failed.");
        }
    }

    PKCS11CardManager(CK_SLOT_ID slotID, CK_FUNCTION_LIST_PTR fl)
    : fl(fl)
    {
        checkError("C_GetTokenInfo", fl->C_GetTokenInfo(slotID, &tokenInfo));
        checkError("C_OpenSession", fl->C_OpenSession(slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session));
        std::vector<CK_OBJECT_HANDLE> objectHandle = findObject(CKO_CERTIFICATE);
        if (objectHandle.empty()) {
            throw std::runtime_error("Could not read cert");
        }

        CK_ATTRIBUTE attribute = {CKA_VALUE, nullptr, 0};
        checkError("C_GetAttributeValue", fl->C_GetAttributeValue(session, objectHandle[0], &attribute, 1));
        signCert.resize(attribute.ulValueLen, 0);
        attribute.pValue = &signCert[0];
        checkError("C_GetAttributeValue", fl->C_GetAttributeValue(session, objectHandle[0], &attribute, 1));
        _log("certificate = %p, certificateLength = %i", &signCert[0], signCert.size());

        const unsigned char* p = &signCert[0];
        cert = d2i_X509(NULL, &p, signCert.size());
    }

    std::vector<unsigned char> getHashWithPadding(const std::vector<unsigned char> &hash) const {
        std::vector<unsigned char> hashWithPadding;
        switch (hash.size()) {
            case BINARY_SHA1_LENGTH:
                hashWithPadding = {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
                break;
            case BINARY_SHA224_LENGTH:
                hashWithPadding = {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
                break;
            case BINARY_SHA256_LENGTH:
                hashWithPadding = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
                break;
            case BINARY_SHA384_LENGTH:
                hashWithPadding = {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
                break;
            case BINARY_SHA512_LENGTH:
                hashWithPadding = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};
                break;
            default:
                _log("incorrect digest length, dropping padding");
        }
        hashWithPadding.insert(hashWithPadding.end(), hash.begin(), hash.end());
        return hashWithPadding;
    }

    PKCS11CardManager(const std::string &module) {
        library = dlopen(module.c_str(), RTLD_LOCAL | RTLD_NOW);
        if (!library) {
            throw std::runtime_error("PKCS11 is not loaded");
        }

        CK_C_GetFunctionList GetFunctionList = (CK_C_GetFunctionList) dlsym(library, "C_GetFunctionList");
        if (dlerror()) {
            dlclose(library);
            library = nullptr;
            throw std::runtime_error("PKCS11 is not loaded");
        }
        checkError("GetFunctionList", GetFunctionList(&fl));
        checkError("C_Initialize", fl->C_Initialize(nullptr));
    }
    
public:

    static PKCS11CardManager* instance(const std::string &module = PKCS11_MODULE) {
        static PKCS11CardManager instance(module);
        return &instance;
    }

    virtual ~PKCS11CardManager() {
        if (session)
            fl->C_CloseSession(session);
        if (cert)
            X509_free(cert);
        if (!library)
            return;
        fl->C_Finalize(nullptr);
        dlclose(library);
    }

    std::vector<CK_SLOT_ID> getAvailableTokens() const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_ULONG slotCount = 0;
        fl->C_GetSlotList(CK_TRUE, nullptr, &slotCount);
        _log("slotCount = %i", slotCount);
        std::vector<CK_SLOT_ID> slotIDs(slotCount, 0);
        fl->C_GetSlotList(CK_TRUE, &slotIDs[0], &slotCount);

        std::vector<CK_SLOT_ID> signingSlotIDs;
        for (CK_ULONG i = 0; i < slotCount; i++) {
            _log("slotID: %i", slotIDs[i]);
            if (i & 1) {
                _log("Found signing slotID: %i", slotIDs[i]);
                signingSlotIDs.push_back(slotIDs[i]);
            }
        }
        return signingSlotIDs;
    }

    PKCS11CardManager *getManagerForReader(CK_SLOT_ID slotId) {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        return new PKCS11CardManager(slotId, fl);
    }

    std::vector<unsigned char> sign(const std::vector<unsigned char> &hash, const PinString &pin) const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_RV result = fl->C_Login(session, CKU_USER, (unsigned char*)pin.c_str(), pin.size());
        switch (result) {
            case CKR_OK:
                break;
            case CKR_FUNCTION_CANCELED:
                throw UserCanceledError();
            case CKR_PIN_INCORRECT:
                throw AuthenticationError();
            case CKR_PIN_LEN_RANGE:
                throw AuthenticationBadInput();
            default:
                checkError("C_Login", result);
        }

        std::vector<CK_OBJECT_HANDLE> privateKeyHandle = findObject(CKO_PRIVATE_KEY);
        if (privateKeyHandle.empty()) {
            throw std::runtime_error("Could not read private key");
        }

        CK_MECHANISM mechanism = {CKM_RSA_PKCS, 0, 0};
        checkError("C_SignInit", fl->C_SignInit(session, &mechanism, privateKeyHandle[0]));
        CK_ULONG signatureLength = 0;
        std::vector<unsigned char> hashWithPadding = getHashWithPadding(hash);
        checkError("C_Sign", fl->C_Sign(session, (CK_BYTE_PTR) &hashWithPadding[0], hashWithPadding.size(), nullptr, &signatureLength));
        std::vector<unsigned char> signature(signatureLength, 0);
        checkError("C_Sign", fl->C_Sign(session, (CK_BYTE_PTR) &hashWithPadding[0], hashWithPadding.size(), &signature[0], &signatureLength));
        checkError("C_Logout", fl->C_Logout(session));

        return signature;
    }

    int getPIN2RetryCount() const {
        if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
        if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
        if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
        return 3;
    }

    std::vector<unsigned char> getSignCert() const {
        return signCert;
    }

    std::string getCN() const {
        return getFromX509Name("commonName");
    }

    std::string getType() const {
        return getFromX509Name("organizationName");
    }

    std::string getCardName() const {
        return getFromX509Name("givenName") + ", " + getFromX509Name("surname");
    }

    std::string getPersonalCode() const {
        return getFromX509Name("serialNumber");
    }

    time_t getValidTo() const {
        if (!cert) {
            throw std::runtime_error("Could not parse cert");
        }
        ASN1_GENERALIZEDTIME *gt = ASN1_TIME_to_generalizedtime(X509_get_notAfter(cert), nullptr);
        std::string timeAsString((const char *) gt->data, gt->length);
        ASN1_GENERALIZEDTIME_free(gt);
        return DateUtils::timeFromStringWithFormat(timeAsString, "%Y%m%d");
    }

    bool isPinpad() const {
        return tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH;
    }
};
