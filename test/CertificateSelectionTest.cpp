/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include <gmock/gmock-spec-builders.h>

#include "gmock/gmock.h"
#include "CertDialog.h"

#include "jsonxx.h"
#include "CertificateSelection.h"
#include "Labels.h"
#include "BinaryUtils.h"
#include "TestUtils.h"

using namespace testing;
using namespace jsonxx;
using namespace std;
using namespace DateUtils;

class MockCertDialog : public CertDialog {
public:
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(hide, void());
  MOCK_METHOD4(addRow, void(int, string, string, string));
  MOCK_METHOD0(getSelectedCertIndex, int());
};

class MockCertificateSelection : public CertificateSelection {
public:
  MockCertificateSelection(CertDialog *certDialog, MockCardManager *manager) : CertificateSelection(certDialog, manager){
  }
  
  MOCK_METHOD0(now, time_t());
};

TEST(CertificateSelection, userCancels) {
  MockCertDialog dialog;
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_CANCEL));
  EXPECT_CALL(dialog, hide());

  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillRepeatedly(Return(TestUtils::expectedAvailableTokens(1)));
  EXPECT_CALL(manager, isCardInReader()).WillOnce(Return(true));
  EXPECT_CALL(manager, getValidTo()).WillOnce(Return(0));

  MockCertificateSelection cert(&dialog, &manager);
  EXPECT_CALL(cert, now()).WillOnce(Return(timeFromString("10.03.2014")));
  
  Object json = cert.getCert();
  ASSERT_EQ(USER_CANCEL, json.get<Number>("returnCode"));
}

TEST(CertificateSelection, onlyValidCertificatesAreAdded) {
  MockCertDialog dialog;
  EXPECT_CALL(dialog, addRow(0, "Igor", "ESTEID", "12.12.2015"));
  EXPECT_CALL(dialog, addRow(1, "Mari-Liis", "DIGI-ID", "10.10.2020"));
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_CLOSE));
  EXPECT_CALL(dialog, hide());

  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillRepeatedly(Return(TestUtils::expectedAvailableTokens(3)));
  EXPECT_CALL(manager, getCN()).WillOnce(Return("Igor")).WillOnce(Return("Mari-Liis"));
  EXPECT_CALL(manager, getType()).WillOnce(Return("ESTEID")).WillOnce(Return("DIGI-ID"));
  EXPECT_CALL(manager, isCardInReader()).WillRepeatedly(Return(true));
  EXPECT_CALL(manager, getValidTo())
      .WillOnce(Return(timeFromString("12.12.2015")))
      .WillOnce(Return(timeFromString("10.10.2020")))
      .WillOnce(Return(timeFromString("12.12.2013")));
  
  MockCertificateSelection cert(&dialog, &manager);
  EXPECT_CALL(cert, now()).WillOnce(Return(timeFromString("10.03.2014")));
  
  Object json = cert.getCert();
  ASSERT_EQ(USER_CANCEL, json.get<Number>("returnCode"));
}

TEST(CertificateSelection, skipReadersWithoutCard) {
  MockCertDialog dialog;
  EXPECT_CALL(dialog, addRow(0, "Igor", "ESTEID", "12.12.2015"));
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_OK));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getSelectedCertIndex()).WillOnce(Return(5));

  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillRepeatedly(Return(TestUtils::expectedAvailableTokens(2)));
  EXPECT_CALL(manager, isCardInReader()).WillOnce(Return(true)).WillOnce(Return(false));
  EXPECT_CALL(manager, getCN()).WillOnce(Return("Igor")).WillOnce(Return("Igor"));
  EXPECT_CALL(manager, getType()).WillOnce(Return("ESTEID")).WillOnce(Return("ESTEID"));
  EXPECT_CALL(manager, getValidTo()).WillOnce(Return(timeFromString("12.12.2015"))).WillOnce(Return(timeFromString("12.12.2015")));
  EXPECT_CALL(manager, getValidFrom()).WillOnce(Return(timeFromString("12.12.2013")));
  EXPECT_CALL(manager, getIssuerCN()).WillOnce(Return("SK"));
  EXPECT_CALL(manager, getCertSerialNumber()).WillOnce(Return("1000"));
  unsigned char certBinary[] = {(unsigned char) 0x30, (unsigned char) 0x31, (unsigned char) 0x32, (unsigned char) 0x33};
  vector<unsigned char> certVector(certBinary, certBinary + 4);
  EXPECT_CALL(manager, getSignCert()).WillOnce(Return(certVector));
  
  MockCertificateSelection cert(&dialog, &manager);
  EXPECT_CALL(cert, now()).WillOnce(Return(timeFromString("10.03.2014")));
  
  Object json = cert.getCert();
  ASSERT_EQ(5, manager.getReaderId());
  ASSERT_EQ("EB62F6B9306DB575C2D596B1279627A4", json.get<string>("id"));
}

TEST(CertificateSelection, getCert) {
  MockCertDialog dialog;
  EXPECT_CALL(dialog, addRow(_, _, _, _));
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_OK));
  EXPECT_CALL(dialog, hide());
  EXPECT_CALL(dialog, getSelectedCertIndex()).WillOnce(Return(5));

  MockCardManager manager;
  EXPECT_CALL(manager, getAvailableTokens()).WillRepeatedly(Return(TestUtils::expectedAvailableTokens(1)));
  EXPECT_CALL(manager, getCN()).WillRepeatedly(Return("Igor"));
  EXPECT_CALL(manager, getType()).WillRepeatedly(Return("ESTEID"));
  EXPECT_CALL(manager, getValidTo()).WillRepeatedly(Return(timeFromString("12.12.2015")));
  EXPECT_CALL(manager, getValidFrom()).WillOnce(Return(timeFromString("12.12.2013")));
  EXPECT_CALL(manager, getIssuerCN()).WillOnce(Return("SK"));
  EXPECT_CALL(manager, getCertSerialNumber()).WillOnce(Return("1000"));
  unsigned char certBinary[] = {(unsigned char) 0x30, (unsigned char) 0x31, (unsigned char) 0x32, (unsigned char) 0x33};
  vector<unsigned char> certVector(certBinary, certBinary + 4);
  EXPECT_CALL(manager, getSignCert()).WillOnce(Return(certVector));
  EXPECT_CALL(manager, isCardInReader()).WillOnce(Return(true));

  MockCertificateSelection cert(&dialog, &manager);
  EXPECT_CALL(cert, now()).WillOnce(Return(timeFromString("10.03.2014")));
  
  Object json = cert.getCert();
  ASSERT_EQ(5, manager.getReaderId());
  ASSERT_EQ("EB62F6B9306DB575C2D596B1279627A4", json.get<string>("id"));
  ASSERT_EQ("30313233", json.get<string>("cert"));
  ASSERT_EQ("12.12.2015", json.get<string>("validTo"));
  ASSERT_EQ("12.12.2013", json.get<string>("validFrom"));
  ASSERT_EQ("Igor", json.get<string>("CN"));
  ASSERT_EQ("ESTEID", json.get<string>("type"));
  ASSERT_EQ("Non-Repudiation", json.get<string>("keyUsage"));
  ASSERT_EQ("SK", json.get<string>("issuerCN"));
  ASSERT_EQ("1000", json.get<string>("certSerialNumber"));
  ASSERT_EQ("30313233", json.get<string>("certificateAsHex"));
}
