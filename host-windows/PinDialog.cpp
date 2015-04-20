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
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(PinDialog, CDialog)

BEGIN_MESSAGE_MAP(PinDialog, CDialog)
	ON_BN_CLICKED(IDOK, &PinDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void PinDialog::OnBnClickedOk() {
	CString rawPin;
	GetDlgItem(IDC_PIN_FIELD)->GetWindowText(rawPin);
	pin = _strdup(ATL::CT2CA(rawPin));
	CDialog::OnOK();
}

char* PinDialog::getPin() {
	return pin;
}
