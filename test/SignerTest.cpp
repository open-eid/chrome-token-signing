/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "Signer.h"
#include <gtkmm.h>
#include <gmock/gmock-spec-builders.h>
#include "jsonxx.h"
#include "Labels.h"
#include "MockCardManager.h"
#include "CardManager.h"
#include "TestUtils.h"

using namespace testing;
using namespace std;
using namespace jsonxx;
using namespace DateUtils;

class MockSigner : public Signer {
public:

  MockSigner(string hash) : Signer(hash, "0") {
  }

  MockSigner(std::string hash, std::string certId, PinDialog *dialog, MockCardManager *manager) : Signer(hash, certId, dialog, manager) {
  }

  MOCK_METHOD2(retriesLeftMessage, void(int, bool));
  MOCK_METHOD0(showPINBlockedMessage, void());
  MOCK_METHOD0(getReaderIdByCertHash, int());
  MOCK_METHOD0(now, time_t());
  
  int parentGetReaderIdByCertHash() {
    return Signer::getReaderIdByCertHash();
  }
};

class MockPinDialog : public PinDialog {
public:
  MOCK_METHOD0(getPin, string());
  MOCK_METHOD0(hide, void());
  MOCK_METHOD1(setCardInfo, void(string));
  MOCK_METHOD1(setErrorMessage, void(string));
};

TEST(Signer, getReaderIdByCertHash) {
  byte correctCertArray[] = {(unsigned char)0x30, (unsigned char)0x31, (unsigned char)0x32, (unsigned char)0x33};
  std::vector<unsigned char> correctCert(correctCertArray, correctCertArray + 4);

  byte wrongCertArray[] = {(unsigned char)0x70, (unsigned char)0x71, (unsigned char)0x72, (unsigned char)0x73};
  std::vector<unsigned char> wrongCert(wrongCertArray, wrongCertArray + 4);
  
  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillOnce(Return(TestUtils::expectedAvailableTokens(2)));
  EXPECT_CALL(manager, isCardInReader()).WillRepeatedly(Return(true));
  EXPECT_CALL(manager, getSignCert()).WillOnce(Return(wrongCert)).WillOnce(Return(correctCert));
  EXPECT_CALL(manager, getValidTo()).WillOnce(Return(timeFromString("01.01.2015")));
    
  MockSigner signer("hash", "EB62F6B9306DB575C2D596B1279627A4", NULL, &manager);
  EXPECT_CALL(signer, now()).WillOnce(Return(timeFromString("10.03.2014")));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Invoke(&signer, &MockSigner::parentGetReaderIdByCertHash));
  
  ASSERT_EQ(1, signer.getReaderIdByCertHash());
}

TEST(Signer, readerForCertHashNotFound) {
  byte correctCertArray[] = {(unsigned char)0x30, (unsigned char)0x31, (unsigned char)0x32, (unsigned char)0x33};
  std::vector<unsigned char> correctCert(correctCertArray, correctCertArray + 4);

  byte wrongCertArray[] = {(unsigned char)0x70, (unsigned char)0x71, (unsigned char)0x72, (unsigned char)0x73};
  std::vector<unsigned char> wrongCert(wrongCertArray, wrongCertArray + 4);
  
  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillOnce(Return(TestUtils::expectedAvailableTokens(2)));
  EXPECT_CALL(manager, isCardInReader()).WillRepeatedly(Return(true));
  EXPECT_CALL(manager, getSignCert()).WillOnce(Return(wrongCert)).WillOnce(Return(correctCert));
    
  MockSigner signer("hash", "WRONG_RANDOM_HASH", NULL, &manager);
  EXPECT_CALL(signer, now()).WillOnce(Return(timeFromString("10.03.2014")));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Invoke(&signer, &MockSigner::parentGetReaderIdByCertHash));
  
  ASSERT_THROW(signer.getReaderIdByCertHash(), CertNotFoundError);
}

TEST(Signer, foundNotValidCertificate) {
  byte correctCertArray[] = {(unsigned char)0x30, (unsigned char)0x31, (unsigned char)0x32, (unsigned char)0x33};
  std::vector<unsigned char> correctCert(correctCertArray, correctCertArray + 4);

  byte wrongCertArray[] = {(unsigned char)0x70, (unsigned char)0x71, (unsigned char)0x72, (unsigned char)0x73};
  std::vector<unsigned char> wrongCert(wrongCertArray, wrongCertArray + 4);
  
  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillOnce(Return(TestUtils::expectedAvailableTokens(2)));
  EXPECT_CALL(manager, isCardInReader()).WillRepeatedly(Return(true));
  EXPECT_CALL(manager, getSignCert()).WillOnce(Return(wrongCert)).WillOnce(Return(correctCert));
  EXPECT_CALL(manager, getValidTo()).WillOnce(Return(timeFromString("12.12.2013")));
    
  MockSigner signer("hash", "EB62F6B9306DB575C2D596B1279627A4", NULL, &manager);
  EXPECT_CALL(signer, now()).WillOnce(Return(timeFromString("10.03.2014")));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Invoke(&signer, &MockSigner::parentGetReaderIdByCertHash));
  
  ASSERT_THROW(signer.getReaderIdByCertHash(), CertNotFoundError);
}

TEST(Signer, error) {
  Signer signer("", "0");
  Object json = signer.error(5);
  
  ASSERT_EQ(5, json.get<Number>("returnCode"));
  ASSERT_STREQ("ID-kaardi lugemine ebaõnnestus", json.get<string>("message").c_str());
}

TEST(Signer, 3retriesLeftInitial) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  EXPECT_CALL(dialog, setErrorMessage(_)).Times(0);
  
  signer.retriesLeftMessage(3, true);
}

TEST(Signer, 3retriesLeft) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  EXPECT_CALL(dialog, setErrorMessage(_)).Times(0);
  
  signer.retriesLeftMessage(3);
}

TEST(Signer, 2retriesLeftInitial) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  
  EXPECT_CALL(dialog, setErrorMessage("Katseid jäänud:2"));
  
  signer.retriesLeftMessage(2, true);
}

TEST(Signer, 2retriesLeft) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  
  EXPECT_CALL(dialog, setErrorMessage("Vale PIN2! Katseid jäänud:2"));
  
  signer.retriesLeftMessage(2);
}

TEST(Signer, 1retryLeftInitial) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  
  EXPECT_CALL(dialog, setErrorMessage("Katseid jäänud:1"));
  
  signer.retriesLeftMessage(1, true);
}

TEST(Signer, 1retryLeft) {
  MockPinDialog dialog;
  Signer signer("hash", "", &dialog, NULL);
  
  EXPECT_CALL(dialog, setErrorMessage("Vale PIN2! Katseid jäänud:1"));
  
  signer.retriesLeftMessage(1);
}

TEST(Signer, noReadersFound) {
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, getAvailableTokens()).WillOnce(Return(TestUtils::expectedAvailableTokens()));
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Invoke(&cardManager, &MockCardManager::parentIsReaderPresent));
  
  MockSigner signer("hash", "", NULL, &cardManager);

  Object signatureJson = signer.sign();
  ASSERT_EQ(READER_NOT_FOUND, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, noCardInReader) {
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getAvailableTokens()).WillOnce(Return(TestUtils::expectedAvailableTokens(1)));
  EXPECT_CALL(cardManager, isCardInReader()).WillOnce(Return(FALSE));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", NULL, &cardManager);
  EXPECT_CALL(signer, now()).WillOnce(Return(timeFromString("10.03.2014")));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Invoke(&signer, &MockSigner::parentGetReaderIdByCertHash));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(CERT_NOT_FOUND, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, getRetryCountFails) {
  MockPinDialog dialog;
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Throw(std::runtime_error("")));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(UNKNOWN_ERROR, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, PIN2AlreadyBlocked) {
  MockPinDialog dialog;
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(0));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);
  EXPECT_CALL(signer, showPINBlockedMessage());
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(USER_CANCEL, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, userBlocksPIN2) {
  MockPinDialog dialog;
  EXPECT_CALL(dialog, setCardInfo("Mari-Liis, 47101010033"));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getPin()).Times(3).WillRepeatedly(Return("12345"));
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(3));
  EXPECT_CALL(cardManager, getCardName()).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(cardManager, getPersonalCode()).WillOnce(Return("47101010033"));
  EXPECT_CALL(cardManager, sign(_, PinString("12345"))).Times(3).WillRepeatedly(Throw(AuthenticationError(false, false)));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);
  EXPECT_CALL(signer, retriesLeftMessage(3, true));
  EXPECT_CALL(signer, retriesLeftMessage(2, false));
  EXPECT_CALL(signer, retriesLeftMessage(1, false));
  EXPECT_CALL(signer, retriesLeftMessage(0, false));
  EXPECT_CALL(signer, showPINBlockedMessage());
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(USER_CANCEL, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, certForHashNotFound) {
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", NULL, &cardManager);
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Throw(CertNotFoundError()));
  
  Object json = signer.sign();
  ASSERT_EQ(CERT_NOT_FOUND, json.get<Number>("returnCode"));
}

TEST(Signer, checkHashSHA1) {
  Signer signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "");
  signer.checkHash();
}

TEST(Signer, checkHashSHA224) {
  Signer signer("bf688a7ca189b1536de55c6e74d6c059ef96b8b6ee4f4f338f18bdf4", "");
  signer.checkHash();
}

TEST(Signer, checkHashSHA256) {
  Signer signer("1c3e17006e570bba9b0c2eae2ad2fc43785435ee70b6438e5b783fcb37886059", "");
  signer.checkHash();
}

TEST(Signer, checkHashSHA512) {
  Signer signer("bfd759b966b2ca1be24a446481ea5ffb7fbb12089d5bf11a104e30641a2b5d7b6668f79489a2f9c87ce38c96e1f718bac3f37626ba50d5f4dfdb8dcaaf9f33e9", "");
  signer.checkHash();
}

TEST(Signer, checkHashWithInvalidHash) {
  Signer signer("Invalid_hash", "");
  ASSERT_THROW(signer.checkHash(), InvalidHashError);
}

TEST(Signer, signWithInvalidHash) {
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  
  MockSigner signer("Invalid hash", "", NULL, &cardManager);
  
  Object json = signer.sign();
  ASSERT_EQ(INVALID_HASH, json.get<Number>("returnCode"));
}

TEST(Signer, sign) {
  MockPinDialog dialog;
  EXPECT_CALL(dialog, setCardInfo("Mari-Liis, 47101010033"));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getPin()).WillOnce(Return("12345"));
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(3));
  EXPECT_CALL(cardManager, getCardName()).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(cardManager, getPersonalCode()).WillOnce(Return("47101010033"));
  EXPECT_CALL(cardManager, isPinpad()).WillRepeatedly(Return(FALSE));
  
  string hash = "FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101";
  byte *hashAsBinaryArray = BinaryUtils::hex2bin(hash.c_str());
  std::vector<unsigned char> hashAsBinary(hashAsBinaryArray, hashAsBinaryArray + (hash.length() / 2));
  byte expectedSignatureArray[] = {(unsigned char)0x30, (unsigned char)0x31, (unsigned char)0x32, (unsigned char)0x33};
  std::vector<unsigned char> expectedSignature(expectedSignatureArray, expectedSignatureArray+4);
  EXPECT_CALL(cardManager, sign(hashAsBinary, PinString("12345"))).WillOnce(Return(expectedSignature));
  
  MockSigner signer(hash, "", &dialog, &cardManager);
  EXPECT_CALL(signer, retriesLeftMessage(_, _));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_STREQ("30313233", signatureJson.get<string>("signature").c_str());
}

TEST(Signer, signingIsCanceledByUser) {
  MockPinDialog dialog;
  EXPECT_CALL(dialog, setCardInfo(_));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getPin()).WillOnce(Throw(UserCanceledError()));

  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(3));
  EXPECT_CALL(cardManager, getCardName()).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(cardManager, getPersonalCode()).WillOnce(Return("47101010033"));
  EXPECT_CALL(cardManager, isPinpad()).WillRepeatedly(Return(FALSE));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);
  EXPECT_CALL(signer, retriesLeftMessage(_, _));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(USER_CANCEL, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, signWithInvalidHashAndCardCombination) {
  MockPinDialog dialog;
  EXPECT_CALL(dialog, setCardInfo(_));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getPin()).WillOnce(Return("12345"));
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(3));
  EXPECT_CALL(cardManager, getCardName()).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(cardManager, getPersonalCode()).WillOnce(Return("47101010033"));
  EXPECT_CALL(cardManager, isPinpad()).WillRepeatedly(Return(FALSE));
  EXPECT_CALL(cardManager, sign(_, _)).WillOnce(Throw(InvalidHashError()));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);
  EXPECT_CALL(signer, retriesLeftMessage(_, _));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(INVALID_HASH, signatureJson.get<Number>("returnCode"));
}

TEST(Signer, signFailsWithUnknownReason) {
  MockPinDialog dialog;
  EXPECT_CALL(dialog, setCardInfo(_));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getPin()).WillOnce(Return("12345"));
  
  MockCardManager cardManager;
  EXPECT_CALL(cardManager, isReaderPresent()).WillOnce(Return(TRUE));
  EXPECT_CALL(cardManager, getPIN2RetryCount()).WillOnce(Return(3));
  EXPECT_CALL(cardManager, getCardName()).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(cardManager, getPersonalCode()).WillOnce(Return("47101010033"));
  EXPECT_CALL(cardManager, isPinpad()).WillRepeatedly(Return(FALSE));
  EXPECT_CALL(cardManager, sign(_, _)).WillOnce(Throw(runtime_error("")));
  
  MockSigner signer("FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", "", &dialog, &cardManager);

  EXPECT_CALL(signer, retriesLeftMessage(_, _));
  EXPECT_CALL(signer, getReaderIdByCertHash()).WillOnce(Return(0));
  
  Object signatureJson = signer.sign();
  ASSERT_EQ(UNKNOWN_ERROR, signatureJson.get<Number>("returnCode"));
}

