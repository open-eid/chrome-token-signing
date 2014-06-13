/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "PinEntryDialog.h"
#include <stdexcept>
#include <gmock/gmock-spec-builders.h>

using namespace testing;
using namespace std;

class MockPinEntryDialog : public PinEntryDialog {
public:
  MOCK_METHOD0(run, int());
  MOCK_METHOD0(readPin, string());
  MOCK_METHOD0(getPin, string());
  
  string parentGetPin() {
    return PinEntryDialog::getPin();
  }
};

TEST(PinEntryDialog, pinDialogIsClosedByPressingCancel) {
  MockPinEntryDialog dialog;
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_CANCEL));
  EXPECT_CALL(dialog, getPin()).WillOnce(Invoke(&dialog, &MockPinEntryDialog::parentGetPin));
  
  ASSERT_THROW(dialog.getPin(), UserCanceledError);
}

TEST(PinEntryDialog, pinDialogIsClosed) {
  MockPinEntryDialog dialog;
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_CLOSE));
  EXPECT_CALL(dialog, getPin()).WillOnce(Invoke(&dialog, &MockPinEntryDialog::parentGetPin));
  
  ASSERT_THROW(dialog.getPin(), UserCanceledError);
}

TEST(PinEntryDialog, pinDialogReturnsPin) {
  MockPinEntryDialog dialog;
  EXPECT_CALL(dialog, run()).WillOnce(Return(GTK_RESPONSE_SIGN));
  EXPECT_CALL(dialog, readPin()).WillOnce(Return("12345"));
  EXPECT_CALL(dialog, getPin()).WillOnce(Invoke(&dialog, &MockPinEntryDialog::parentGetPin));
  
  ASSERT_EQ("12345", dialog.getPin());
}