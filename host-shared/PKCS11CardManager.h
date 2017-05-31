/*
 * Chrome Token Signing Native Host
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

#pragma once

#include "pkcs11.h"
#include "Logger.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <afx.h> //Using afx.h instead of windows.h because of MFC
#else
#include <dlfcn.h>
#endif

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

#define C(API, ...) Call(__FILE__, __LINE__, "C_"#API, fl->C_##API, __VA_ARGS__)

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

class PKCS11TokenNotRecognized: public std::runtime_error {
public:
    PKCS11TokenNotRecognized() : std::runtime_error("Token not recognized.") {}
};

class PKCS11TokenNotPresent: public std::runtime_error {
public:
    PKCS11TokenNotPresent() : std::runtime_error("Token not present.") {}
};

class PKCS11CardManager {
private:
#ifdef _WIN32
    HINSTANCE library = 0;
#else
    void *library = nullptr;
#endif
    CK_FUNCTION_LIST_PTR fl = nullptr;

    template <typename Func, typename... Args>
    void Call(const char *file, int line, const char *function, Func func, Args... args) const
    {
        CK_RV rv = func(args...);
        Logger::writeLog(function, file, line, "return value %u", rv);
        switch (rv) {
            case CKR_OK:
                break;
            case CKR_FUNCTION_CANCELED:
                throw UserCanceledError();
            case CKR_PIN_INCORRECT:
                throw AuthenticationError();
            case CKR_PIN_LEN_RANGE:
                throw AuthenticationBadInput();
            case CKR_TOKEN_NOT_RECOGNIZED:
                throw PKCS11TokenNotRecognized();
            case CKR_TOKEN_NOT_PRESENT:
                throw PKCS11TokenNotPresent();
            default:
                throw std::runtime_error("PKCS11 method failed.");
        }
    }

    std::vector<CK_OBJECT_HANDLE> findObject(CK_SESSION_HANDLE session, CK_OBJECT_CLASS objectClass, CK_ULONG max = 2) const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_ATTRIBUTE searchAttribute = {CKA_CLASS, &objectClass, sizeof(objectClass)};
        C(FindObjectsInit, session, &searchAttribute, 1);
        CK_ULONG objectCount = max;
        std::vector<CK_OBJECT_HANDLE> objectHandle(objectCount);
        C(FindObjects, session, objectHandle.data(), objectHandle.size(), &objectCount);
        C(FindObjectsFinal, session);
        objectHandle.resize(objectCount);
        return objectHandle;
    }

    PKCS11CardManager(const std::string &module) {
        CK_C_GetFunctionList C_GetFunctionList = nullptr;
#ifdef _WIN32
        library = LoadLibraryA(module.c_str());
        if (library)
            C_GetFunctionList = CK_C_GetFunctionList(GetProcAddress(library, "C_GetFunctionList"));
#else
        library = dlopen(module.c_str(), RTLD_LOCAL | RTLD_NOW);
        if (library)
            C_GetFunctionList = CK_C_GetFunctionList(dlsym(library, "C_GetFunctionList"));
#endif

        if (!C_GetFunctionList) {
            _log("Function List not loaded %s", module.c_str());
            throw std::runtime_error("PKCS11 is not loaded");
        }
        Call(__FILE__, __LINE__, "C_GetFunctionList", C_GetFunctionList, &fl);
        _log("initializing module %s", module.c_str());
        C(Initialize, nullptr);
    }

public:

    static PKCS11CardManager* instance(const std::string &module = PKCS11_MODULE) {
        static PKCS11CardManager instance(module);
        return &instance;
    }

    ~PKCS11CardManager() {
        if (!library)
            return;
        C(Finalize, nullptr);
#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
    }

    struct Token {
        std::string label;
        CK_SLOT_ID slotID;
        std::vector<unsigned char> cert;
        size_t index;
        int retry;
        bool pinpad;
        unsigned long minPinLen, maxPinLen;
    };

    std::vector<Token> tokens() const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_ULONG slotCount = 0;
        C(GetSlotList, CK_TRUE, nullptr, &slotCount);
        _log("slotCount = %i", slotCount);
        std::vector<CK_SLOT_ID> slotIDs(slotCount, 0);
        C(GetSlotList, CK_TRUE, slotIDs.data(), &slotCount);
        std::reverse(slotIDs.begin(), slotIDs.end());

        std::vector<Token> result;
        for (CK_SLOT_ID slotID : slotIDs)
        {
            CK_TOKEN_INFO tokenInfo;
            C(GetTokenInfo, slotID, &tokenInfo);
            CK_SESSION_HANDLE session = 0;
            C(OpenSession, slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session);

            std::vector<CK_OBJECT_HANDLE> objs = findObject(session, CKO_CERTIFICATE);
            for (size_t i = 0; i < objs.size(); ++i) {
                CK_ATTRIBUTE attribute = { CKA_VALUE, nullptr, 0 };
                C(GetAttributeValue, session, objs[i], &attribute, 1);
                std::vector<unsigned char> cert(attribute.ulValueLen, 0);
                attribute.pValue = cert.data();
                C(GetAttributeValue, session, objs[i], &attribute, 1);
                result.push_back({ std::string((const char*)tokenInfo.label, sizeof(tokenInfo.label)), slotID, cert, i,
                    [&] {
                        if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
                        if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
                        if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
                        return 3;
                    }(),
                    bool(tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH),
                    tokenInfo.ulMinPinLen,
                    tokenInfo.ulMaxPinLen,
                });
            }

            C(CloseSession, session);
        }
        return result;
    }

    std::vector<unsigned char> sign(const Token &token, const std::vector<unsigned char> &hash, const char *pin) const {
        if (!fl) {
            throw std::runtime_error("PKCS11 is not loaded");
        }
        CK_SESSION_HANDLE session = 0;
        C(OpenSession, token.slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session);
        C(Login, session, CKU_USER, (unsigned char*)pin, pin ? strlen(pin) : 0);
        std::vector<CK_OBJECT_HANDLE> privateKeyHandle = findObject(session, CKO_PRIVATE_KEY);
        if (privateKeyHandle.empty()) {
            throw std::runtime_error("Could not read private key");
        }
        CK_MECHANISM mechanism = {CKM_RSA_PKCS, 0, 0};
        _log("found %i private keys in slot, using key in position %i", privateKeyHandle.size(), token.index);
        C(SignInit, session, &mechanism, privateKeyHandle[token.index]);
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
        CK_ULONG signatureLength = 0;
        C(Sign, session, hashWithPadding.data(), hashWithPadding.size(), nullptr, &signatureLength);
        std::vector<unsigned char> signature(signatureLength, 0);
        C(Sign, session, hashWithPadding.data(), hashWithPadding.size(), signature.data(), &signatureLength);
        C(Logout, session);
        C(CloseSession, session);

        return signature;
    }
};
