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
#include "Labels.h"
#include "Logger.h"

using namespace std;

DialogManager::DialogManager() {
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

char* DialogManager::getPin() {
	_log("Showing pin entry dialog");
	PinDialog dialog;
	if (dialog.DoModal() != IDOK) {
		_log("User cancelled");
		throw UserCancelledException();
	}
	return dialog.getPin();
}

void DialogManager::showWrongPinError(int triesLeft) {
	_log("Showing incorrect pin error dialog, %i tries left", triesLeft);
	wstring msg = Labels::l10n.get("tries left") + L" " + to_wstring(triesLeft);
	MessageBox(NULL, msg.c_str(), Labels::l10n.get("incorrect PIN2").c_str(), MB_OK | MB_ICONERROR);
}

void DialogManager::showPinBlocked() {
	_log("Showing pin blocked dialog");
	MessageBox(NULL, Labels::l10n.get("PIN2 blocked").c_str(), L"PIN Blocked", MB_OK | MB_ICONERROR);
}
