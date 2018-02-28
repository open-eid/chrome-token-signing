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
#include "Exceptions.h"

#include <algorithm>
#include <cstring>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <afx.h> //Using afx.h instead of windows.h because of MFC
#else
#include <dlfcn.h>
#endif

#define C(API, ...) Call(__FILE__, __LINE__, "C_"#API, fl->C_##API, __VA_ARGS__)

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
                throw UserCancelledException();
            case CKR_PIN_INCORRECT:
                throw AuthenticationError();
            case CKR_PIN_LEN_RANGE:
                throw AuthenticationBadInput();
            case CKR_TOKEN_NOT_RECOGNIZED:
                throw PKCS11TokenNotRecognized();
            case CKR_TOKEN_NOT_PRESENT:
                throw PKCS11TokenNotPresent();
            default:
                throw PKCS11Exception("PKCS11 method failed.");
        }
    }

    std::vector<CK_BYTE> attribute(CK_SESSION_HANDLE session, CK_OBJECT_CLASS obj, CK_ATTRIBUTE_TYPE attr) const {
        CK_ATTRIBUTE attribute = { attr, nullptr, 0 };
        C(GetAttributeValue, session, obj, &attribute, 1);
        std::vector<CK_BYTE> data(attribute.ulValueLen, 0);
        attribute.pValue = data.data();
        C(GetAttributeValue, session, obj, &attribute, 1);
        return data;
    }

    std::vector<CK_OBJECT_HANDLE> findObject(CK_SESSION_HANDLE session, CK_OBJECT_CLASS objectClass, const std::vector<CK_BYTE> &id = std::vector<CK_BYTE>()) const {
        if (!fl)
            throw DriverException();
        CK_BBOOL btrue = CK_TRUE;
        std::vector<CK_ATTRIBUTE> searchAttribute{
            {CKA_CLASS, &objectClass, CK_ULONG(sizeof(objectClass))},
            {CKA_TOKEN, &btrue, CK_ULONG(sizeof(btrue))}
        };
        if (!id.empty())
            searchAttribute.push_back({ CKA_ID, CK_VOID_PTR(id.data()), CK_ULONG(id.size()) });
        C(FindObjectsInit, session, searchAttribute.data(), CK_ULONG(searchAttribute.size()));
        CK_ULONG objectCount = 32;
        std::vector<CK_OBJECT_HANDLE> objectHandle(objectCount);
        C(FindObjects, session, objectHandle.data(), CK_ULONG(objectHandle.size()), &objectCount);
        C(FindObjectsFinal, session);
        objectHandle.resize(objectCount);
        return objectHandle;
    }

public:
    PKCS11CardManager(const std::string &module) {
        CK_C_GetFunctionList C_GetFunctionList = nullptr;
        std::string error;
#ifdef _WIN32
        library = LoadLibraryA(module.c_str());
        if (library)
            C_GetFunctionList = CK_C_GetFunctionList(GetProcAddress(library, "C_GetFunctionList"));
		else
		{
			LPSTR msg = nullptr;
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), LPSTR(&msg), 0, nullptr);
			error = msg;
			LocalFree(msg);
		}
#else
        library = dlopen(module.c_str(), RTLD_LOCAL | RTLD_NOW);
        if (library)
            C_GetFunctionList = CK_C_GetFunctionList(dlsym(library, "C_GetFunctionList"));
        else
            error = dlerror();
#endif

        if (!C_GetFunctionList) {
            _log("Function List not loaded %s: %s", module.c_str(), error.c_str());
            throw DriverException();
        }
        Call(__FILE__, __LINE__, "C_GetFunctionList", C_GetFunctionList, &fl);
        _log("initializing module %s", module.c_str());
        C(Initialize, nullptr);
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
        std::vector<CK_BYTE> cert, certID;
        int retry;
        bool pinpad;
        CK_ULONG minPinLen, maxPinLen;
    };

    std::vector<Token> tokens() const {
        if (!fl)
            throw DriverException();
        CK_ULONG slotCount = 0;
        C(GetSlotList, CK_TRUE, nullptr, &slotCount);
        _log("slotCount = %i", slotCount);
        std::vector<CK_SLOT_ID> slotIDs(slotCount, 0);
        C(GetSlotList, CK_TRUE, slotIDs.data(), &slotCount);

        std::vector<Token> result;
        for (CK_SLOT_ID slotID : slotIDs)
        {
            CK_TOKEN_INFO tokenInfo;
            try {
                C(GetTokenInfo, slotID, &tokenInfo);
            } catch(const BaseException &e) {
                _log("Failed to get slot info at SLOT ID %u '%s', skiping", slotID, e.what());
                continue;
            }
            CK_SESSION_HANDLE session = 0;
            C(OpenSession, slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session);

            for (CK_OBJECT_HANDLE obj: findObject(session, CKO_CERTIFICATE)) {
                result.push_back({ std::string((const char*)tokenInfo.label, sizeof(tokenInfo.label)), slotID,
                    attribute(session, obj, CKA_VALUE),
                    attribute(session, obj, CKA_ID),
                    [&] {
                        if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
                        if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
                        if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
                        return 3;
                    }(),
                    (tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH) > 0,
                    tokenInfo.ulMinPinLen,
                    tokenInfo.ulMaxPinLen,
                });
            }

            C(CloseSession, session);
        }
        return result;
    }

    std::vector<CK_BYTE> sign(const Token &token, const std::vector<CK_BYTE> &hash, const char *pin) const {
        if (!fl)
            throw DriverException();
        CK_SESSION_HANDLE session = 0;
        C(OpenSession, token.slotID, CKF_SERIAL_SESSION, nullptr, nullptr, &session);
        C(Login, session, CKU_USER, CK_CHAR_PTR(pin), CK_ULONG(pin ? strlen(pin) : 0));
        if (token.certID.empty())
            throw PKCS11Exception("Could not read private key. Certificate ID is empty");
        std::vector<CK_OBJECT_HANDLE> privateKeyHandle = findObject(session, CKO_PRIVATE_KEY, token.certID);
        if (privateKeyHandle.empty())
            throw PKCS11Exception("Could not read private key. Key not found");
        if (privateKeyHandle.size() > 1)
            throw PKCS11Exception("Could not read private key. Found multiple keys");
        _log("found %i private keys in slot, using key ID %x", privateKeyHandle.size(), token.certID.data());

        CK_KEY_TYPE keyType = CKK_RSA;
        CK_ATTRIBUTE attribute = { CKA_KEY_TYPE, &keyType, sizeof(keyType) };
        C(GetAttributeValue, session, privateKeyHandle[0], &attribute, 1);
        
        CK_MECHANISM mechanism = {keyType == CKK_ECDSA ? CKM_ECDSA : CKM_RSA_PKCS, 0, 0};
        C(SignInit, session, &mechanism, privateKeyHandle[0]);
        std::vector<CK_BYTE> hashWithPadding;
        if (keyType == CKK_RSA) {
            switch (hash.size()) {
                case 20: //BINARY_SHA1_LENGTH:
                    hashWithPadding = {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
                    break;
                case 28: //BINARY_SHA224_LENGTH:
                    hashWithPadding = {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
                    break;
                case 32: //BINARY_SHA256_LENGTH:
                    hashWithPadding = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
                    break;
                case 48: //BINARY_SHA384_LENGTH:
                    hashWithPadding = {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
                    break;
                case 64: //BINARY_SHA512_LENGTH:
                    hashWithPadding = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};
                    break;
                default:
                    _log("incorrect digest length");
                    throw InvalidHashException();
            }
        }
        hashWithPadding.insert(hashWithPadding.end(), hash.begin(), hash.end());
        CK_ULONG signatureLength = 0;
        C(Sign, session, hashWithPadding.data(), CK_ULONG(hashWithPadding.size()), nullptr, &signatureLength);
        std::vector<CK_BYTE> signature(signatureLength, 0);
        C(Sign, session, hashWithPadding.data(), CK_ULONG(hashWithPadding.size()), signature.data(), &signatureLength);
        C(Logout, session);
        C(CloseSession, session);

        return signature;
    }
};
