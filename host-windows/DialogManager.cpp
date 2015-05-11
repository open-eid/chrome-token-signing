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

#include "DialogManager.h"
#include "PinDialog.h"
#include "HostExceptions.h"
#include "Logger.h"
#include <string>

using namespace std;

void initializeMFC() {
	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule == NULL) {
		_log("MFC initialization failed. Module handle is null");
		throw TechnicalException("MFC initialization failed. Module handle is null");
	}
	// initialize MFC
	if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0)) {
		_log("MFC initialization failed");
		throw TechnicalException("MFC initialization failed");
	}
}

wstring getWrongPinErrorMessage(int triesLeft) {
	wstring msg = L"An incorrect PIN was entered. You have ";
	if (triesLeft == 1) {
		return msg + L"1 retry left!";
	}
	return msg + to_wstring(triesLeft) + L" retries left!";
}

char* DialogManager::getPin() {
	_log("Showing pin entry dialog");
	initializeMFC();
	PinDialog dialog;
	INT_PTR nResponse = dialog.DoModal();
	if (nResponse == IDOK) {
		return dialog.getPin();
	}
	else {
		_log("User cancelled");
		throw UserCancelledException();
	}
}

void DialogManager::showWrongPinError(int triesLeft) {
	_log("Showing incorrect pin error dialog, %i tries left", triesLeft);
	wstring msg = getWrongPinErrorMessage(triesLeft);
	MessageBox(NULL, msg.c_str(), L"Wrong PIN", MB_OK | MB_ICONERROR);
}

void DialogManager::showPinBlocked() {
	_log("Showing pin blocked dialog");
	MessageBox(NULL, L"You have entered wrong PIN for too many times. Your PIN is blocked.", L"PIN Blocked", MB_OK | MB_ICONERROR);
}
