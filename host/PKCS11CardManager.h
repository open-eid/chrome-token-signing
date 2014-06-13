/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "CardManager.h"
#include "pkcs11.h"
#include <dlfcn.h>
#include "Logger.h"
#include "DateUtils.h"
#include <asn1.h>
#include "error.h"
#include "BinaryUtils.h"

#ifndef PKCS11CARDMANAGER_H
#define	PKCS11CARDMANAGER_H

CK_BYTE RSA_SHA1_DESIGNATOR_PREFIX[] = {48, 33, 48, 9, 6, 5, 43, 14, 3, 2, 26, 5, 0, 4, 20};
CK_BYTE RSA_SHA224_DESIGNATOR_PREFIX[] = {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
CK_BYTE RSA_SHA256_DESIGNATOR_PREFIX[] = {48, 49, 48, 13, 6, 9, 96, 134, 72, 1, 101, 3, 4, 2, 1, 5, 0, 4, 32};
CK_BYTE RSA_SHA512_DESIGNATOR_PREFIX[] = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};

class PKCS11CardManager : public CardManager {
 private:
	bool isMainManager;
	void *library;
	std::vector<unsigned int> signingSlotIDs;
	X509 *cert;
	X509_name_st *subject;
	X509_name_st *issuer;
	CK_SESSION_HANDLE session;
	CK_SLOT_ID slotID;
	CK_FUNCTION_LIST_PTR fl;
	CK_TOKEN_INFO tokenInfo;
	std::vector<unsigned char> signCert;

	void *loadModule(std::string moduleName) {
		void *library = dlopen(moduleName.c_str(), RTLD_LOCAL | RTLD_NOW);
		if (!library) {
			_log("Failed to load module: ", moduleName.c_str());
			return NULL;
		}

		CK_C_GetFunctionList GetFunctionList = (CK_C_GetFunctionList) dlsym(library, "C_GetFunctionList");
		const char *error = dlerror();
		if (error) {
			_log("Cannot load symbol 'hello': ", error);
			dlclose(library);
			return NULL;
		}
		GetFunctionList(&fl);

		return library;
	}

	void initSlotIDs() {
		CK_ULONG slotCount;
		fl->C_GetSlotList(CK_TRUE, NULL_PTR, &slotCount);
		_log("slotCount = %i", slotCount);
		CK_SLOT_ID_PTR slotIDs = (CK_SLOT_ID_PTR) malloc(sizeof(CK_SLOT_ID) * slotCount);
		fl->C_GetSlotList(CK_TRUE, slotIDs, &slotCount);
		for (int i = 0; i < slotCount; i++) {
			_log("slotID: %i", slotIDs[i]);
			if (i & 1) {
				_log("Found signing slotID: %i", slotIDs[i]);
				signingSlotIDs.push_back(slotIDs[i]);
			}
		}
		free(slotIDs);
	}

	std::string getFromX509Name(X509_name_st *name, std::string subjectField) {
		char *X509Value = (char*)malloc(1024);
		int length = X509_NAME_get_text_by_NID(name, OBJ_txt2nid(subjectField.c_str()), X509Value, 1024);
		_log("%s length is %i (%i), %p", subjectField.c_str(), length, strlen(X509Value), X509Value);
		std::string value(X509Value);
		free(X509Value);
		
		return value;
	}

	time_t ASN1TimeToTime_t(ASN1_TIME *time1) {
		ASN1_GENERALIZEDTIME *gt = ASN1_TIME_to_generalizedtime(time1, NULL_PTR);
		std::string timeAsString((const char *) gt->data, gt->length);
		time_t convertedTime = DateUtils::timeFromStringWithFormat(timeAsString, "%Y%m%d");
		  
	  free(gt);
		return convertedTime;
	}
	
	void checkError(std::string methodName, CK_RV methodResult) {
		if (methodResult != CKR_OK) {
			_log("%s return value %i", methodName.c_str(), methodResult);
			throw std::runtime_error("PKCS11 method failed.");
		}
	}
	
	PKCS11CardManager(CK_SLOT_ID slotId, CK_FUNCTION_LIST_PTR fl) {
		this->fl = fl;
		slotID = slotId;
		session = 0;
		cert = NULL;
		isMainManager = false;
		subject = issuer = NULL;
		checkError("C_GetTokenInfo", fl->C_GetTokenInfo(slotID, &tokenInfo));

		CK_OBJECT_CLASS objectClass = CKO_CERTIFICATE;
		CK_OBJECT_HANDLE objectHandle;
		CK_ULONG objectCount;
		CK_ATTRIBUTE searchAttribute = {CKA_CLASS, &objectClass, sizeof(objectClass)};
		CK_ATTRIBUTE attribute = {CKA_VALUE, NULL_PTR, 0};
		CK_ULONG certificateLength;
		CK_BYTE_PTR certificate;

		
		checkError("C_OpenSession", fl->C_OpenSession(slotID, CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &session));
		checkError("C_FindObjectsInit", fl->C_FindObjectsInit(session, &searchAttribute, 1));
		checkError("C_FindObjects", fl->C_FindObjects(session, &objectHandle, 1, &objectCount));
		checkError("C_FindObjectsFinal", fl->C_FindObjectsFinal(session));

		if (objectCount == 0) {
			throw std::runtime_error("Could not read cert");
		}
		
		checkError("C_GetAttributeValue", fl->C_GetAttributeValue(session, objectHandle, &attribute, 1));
		certificateLength = attribute.ulValueLen;
		certificate = (CK_BYTE_PTR) malloc(certificateLength);
		attribute.pValue = certificate;
		checkError("C_GetAttributeValue", fl->C_GetAttributeValue(session, objectHandle, &attribute, 1));
		_log("certificate = %p, certificateLength = %i", certificate, certificateLength);

		getX509Cert(certificate, certificateLength);

		_log("ASN X509 cert: %p", cert);
		signCert = std::vector<unsigned char>(certificate, certificate + certificateLength);
		free(certificate);
	}
	
	void getX509Cert(unsigned char* certificate, int certificateLength) {
		const unsigned char* p = certificate;
		cert = d2i_X509(NULL, &p, certificateLength);
		subject = X509_get_subject_name(cert);
		issuer = X509_get_issuer_name(cert);
	}

 public:
	PKCS11CardManager(CK_TOKEN_INFO *tokenInfo, std::string certAsHEX) {
	  unsigned char *certificate = BinaryUtils::hex2bin(certAsHEX.c_str());
		getX509Cert(certificate, certAsHEX.length()/2);
		free(certificate);
		this->tokenInfo = *tokenInfo;
		isMainManager = false;
		session = 0;
	}

	PKCS11CardManager() {
		library = loadModule("opensc-pkcs11.so");
		fl->C_Initialize(NULL_PTR);
		initSlotIDs();
		session = 0;
		cert = NULL;
		isMainManager = true;
		subject = issuer = NULL;
	}

	~PKCS11CardManager() {
		if (session) {
			fl->C_CloseSession(session);
			session = 0;
		}
		/*if (issuer) {
			free(issuer);
			issuer = NULL;
		}
		if (subject){
			free(subject);
			subject = NULL;
		}
		if(cert) {
			X509_free (cert);
			cert = NULL;
		}*/

		if (isMainManager) {
			dlclose(library);
		}
	}

	std::vector<unsigned int> getAvailableTokens() {
		return signingSlotIDs;
	}

	bool isCardInReader() {
		CK_RV result = fl->C_GetTokenInfo(slotID, &tokenInfo);
		if (result == CKR_DEVICE_REMOVED) {
			return false;
		}
		checkError("C_GetTokenInfo", result);
		return true;
	}

	PKCS11CardManager *getManagerForReader(int slotId) {
		PKCS11CardManager *manager = new PKCS11CardManager(slotId, fl);
		return manager;
	}

	std::vector<unsigned char> getHashWithPadding(const std::vector<unsigned char> &hash) {
		std::vector<unsigned char> hashWithPadding;
		switch (hash.size()) {
		case BINARY_SHA1_LENGTH:
			hashWithPadding = std::vector<unsigned char>(RSA_SHA1_DESIGNATOR_PREFIX, RSA_SHA1_DESIGNATOR_PREFIX + sizeof(RSA_SHA1_DESIGNATOR_PREFIX));
			break;
		case BINARY_SHA224_LENGTH:
			hashWithPadding = std::vector<unsigned char>(RSA_SHA224_DESIGNATOR_PREFIX, RSA_SHA224_DESIGNATOR_PREFIX + sizeof(RSA_SHA224_DESIGNATOR_PREFIX));
			break;
		case BINARY_SHA256_LENGTH:
			hashWithPadding = std::vector<unsigned char>(RSA_SHA256_DESIGNATOR_PREFIX, RSA_SHA256_DESIGNATOR_PREFIX + sizeof(RSA_SHA256_DESIGNATOR_PREFIX));
			break;
		case BINARY_SHA512_LENGTH:
			hashWithPadding = std::vector<unsigned char>(RSA_SHA512_DESIGNATOR_PREFIX, RSA_SHA512_DESIGNATOR_PREFIX + sizeof(RSA_SHA512_DESIGNATOR_PREFIX));
			break;
		default:
			_log("incorrect digest length, dropping padding");
		}
		hashWithPadding.insert(hashWithPadding.end(), hash.begin(), hash.end());
		return hashWithPadding;
	}
	
	std::vector<unsigned char> sign(const std::vector<unsigned char> &hash, const PinString &pin) {
		CK_OBJECT_HANDLE privateKeyHandle;
		CK_ULONG objectCount;
		CK_MECHANISM mechanism = {CKM_RSA_PKCS, 0, 0};
		CK_OBJECT_CLASS objectClass = CKO_PRIVATE_KEY;
		CK_ATTRIBUTE searchAttribute = {CKA_CLASS, &objectClass, sizeof(objectClass)};
	
		CK_RV result = fl->C_Login(session, CKU_USER, (unsigned char*)pin.c_str(), pin.size());
		if (result == CKR_PIN_LEN_RANGE) {
			throw AuthenticationErrorBadInput();
		}
		if (result == CKR_FUNCTION_CANCELED) {
			throw AuthenticationErrorAborted();
		}
		if (result == CKR_PIN_INCORRECT) {
			throw AuthenticationError();
		}
		checkError("C_Login", result);
		checkError("C_FindObjectsInit", fl->C_FindObjectsInit(session, &searchAttribute, 1));
		checkError("C_FindObjects", fl->C_FindObjects(session, &privateKeyHandle, 1, &objectCount));
		checkError("C_FindObjectsFinal", fl->C_FindObjectsFinal(session));

		if (objectCount == 0) {
			throw std::runtime_error("Could not read private key");
		}

		checkError("C_SignInit", fl->C_SignInit(session, &mechanism, privateKeyHandle));
		CK_ULONG signatureLength;
		
		std::vector<unsigned char> hashWithPadding = getHashWithPadding(hash);
		checkError("C_Sign", fl->C_Sign(session, (CK_BYTE_PTR) &hashWithPadding[0], hashWithPadding.size(), NULL, &signatureLength));
		CK_BYTE_PTR signature = (CK_BYTE_PTR) malloc(signatureLength);
		
		result = fl->C_Sign(session, (CK_BYTE_PTR) &hashWithPadding[0], hashWithPadding.size(), signature, &signatureLength);
		if (result == CKR_USER_NOT_LOGGED_IN) {
			throw InvalidHashError();
		}
		checkError("C_Sign", result);
		checkError("C_Logout", fl->C_Logout(session));
		
		std::vector<unsigned char> resultSignature(signature, signature + signatureLength);
		free(signature);
		return resultSignature;
	}

	std::string getCardName() {
		return getFromX509Name(subject, "givenName") + " " + getFromX509Name(subject, "surname");
	}

	std::string getPersonalCode() {
		return getFromX509Name(subject, "serialNumber");
	}

	int getPIN2RetryCount() {
		if (tokenInfo.flags & CKF_USER_PIN_LOCKED) return 0;
		else if (tokenInfo.flags & CKF_USER_PIN_FINAL_TRY) return 1;
		else if (tokenInfo.flags & CKF_USER_PIN_COUNT_LOW) return 2;
		else return 3;
	}

	std::vector<unsigned char> getSignCert() {
		return signCert;
	}

	std::string getCN() {
		return getFromX509Name(subject, "commonName");
	}

	std::string getType() {
		return getFromX509Name(subject, "organizationName");
	}

	time_t getValidTo() {
		ASN1_TIME *time = X509_get_notAfter(cert);
		time_t validTo = ASN1TimeToTime_t(time);
		free(time);
		return validTo;
	}

	time_t getValidFrom() {
		ASN1_TIME *time = X509_get_notBefore(cert);
		time_t validFrom = ASN1TimeToTime_t(time);
		free(time);
		return validFrom;
	}

	std::string getIssuerCN() {
		return getFromX509Name(issuer, "commonName");
	}

	std::string getCertSerialNumber() {
		ASN1_INTEGER *serial = X509_get_serialNumber(cert);
		BIGNUM *serialBn = ASN1_INTEGER_to_BN(serial, NULL);
		char *serialHex = BN_bn2hex(serialBn);

		std::string serialString = std::string(serialHex);
		free(serial);
		free(serialBn);
		free(serialHex);
		return serialString;
	}

	bool isPinpad() {
		return(tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH);
	}
};

#endif	/* PKCS11CARDMANAGER_H */

