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

#include "PinDialog.h"

#include "Labels.h"
#include "PinDialogResource.h"

std::string PinDialog::getPin(const std::wstring &label, HWND pParent)
{
	PinDialog p;
	p.label = label;
	if (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_PIN_DIALOG), pParent, DlgProc, LPARAM(&p)) != IDOK)
		p.pin.clear();
	return p.pin;
}

INT_PTR CALLBACK PinDialog::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		PinDialog *self = (PinDialog*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SetDlgItemText(hwndDlg, IDC_PIN_MESSAGE, self->label.c_str());
		SetDlgItemText(hwndDlg, IDCANCEL, Labels::l10n.get("cancel").c_str());
		SendMessage(hwndDlg, DM_SETDEFID, IDCANCEL, 0);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PIN_FIELD:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				WORD len = WORD(SendDlgItemMessage(hwndDlg, IDC_PIN_FIELD, EM_LINELENGTH, 0, 0));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), len >= 4);
				SendMessage(hwndDlg, DM_SETDEFID, len >= 4 ? IDOK : IDCANCEL, 0);
			}
			break;
		case IDOK:
		{
			size_t len = size_t(SendDlgItemMessage(hwndDlg, IDC_PIN_FIELD, EM_LINELENGTH, 0, 0));
			std::wstring tmp(len + 1, 0);
			GetDlgItemText(hwndDlg, IDC_PIN_FIELD, &tmp[0], tmp.size());
			PinDialog *self = (PinDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			self->pin = std::string(tmp.cbegin(), tmp.cend());
			return EndDialog(hwndDlg, IDOK);
		}
		case IDCANCEL:
			return EndDialog(hwndDlg, IDCANCEL);
		}
		return FALSE;
	}
	return FALSE;
}
