// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2012-2013 - TortoiseGit
// Copyright (C) 2003-2008,2010 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "resource.h"			//if you defined some IDS_MSGBOX_xxxx this include is needed!
#include "messagebox.h"
#include "ClipboardHelper.h"
#include "SmartHandle.h"

CMessageBox::CMessageBox(void)
	: m_hIcon(nullptr)
	, m_uButton1Ret(1)
	, m_uButton2Ret(2)
	, m_uButton3Ret(3)
	, m_uCancelRet(0)
	, m_bShowCheck(FALSE)
	, m_bDestroyIcon(FALSE)
	, m_nDefButton(0)
	, m_uType(0)
	, m_bChecked(FALSE)
{
	SecureZeroMemory(&m_LogFont, sizeof(LOGFONT));
}

CMessageBox::~CMessageBox(void)
{
	if (m_bDestroyIcon)
		::DestroyIcon(m_hIcon);
}

UINT CMessageBox::ShowCheck(HWND hWnd, UINT nMessage, UINT nCaption, int nDef, LPCTSTR icon, UINT nButton1, UINT nButton2, UINT nButton3, LPCTSTR lpRegistry, UINT nCheckMessage/* = nullptr*/, BOOL* bChecked)
{
	CString sButton1;
	CString sButton2;
	CString sButton3;
	CString sMessage;
	CString sCaption;
	CString nCheckMsg;
	sButton1.LoadString(nButton1);
	sButton2.LoadString(nButton2);
	sButton3.LoadString(nButton3);
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	nCheckMsg.LoadString(nCheckMessage);
	return CMessageBox::ShowCheck(hWnd, sMessage, sCaption, nDef, icon, sButton1, sButton2, sButton3, lpRegistry, nCheckMsg, bChecked);
}

UINT CMessageBox::ShowCheck(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, int nDef, LPCTSTR icon, LPCTSTR lpButton1, LPCTSTR lpButton2, LPCTSTR lpButton3, LPCTSTR lpRegistry, LPCTSTR lpCheckMessage/* = nullptr*/, BOOL* bChecked)
{
	//check the registry if we have to show the box or just return with the last used return value
	//this would be the case if the user pressed "do not show again".
	DWORD dwRetVal;
	HKEY hKey;
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\TortoiseGit\\";
	path += AfxGetApp()->m_pszProfileName;
#endif
	if (lpRegistry && *lpRegistry && RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_EXECUTE, &hKey)==ERROR_SUCCESS)
	{
		int size = sizeof(dwRetVal);
		DWORD type;
		if (RegQueryValueEx(hKey, lpRegistry, nullptr, &type, (BYTE*)&dwRetVal,(LPDWORD)&size) == ERROR_SUCCESS)
		{
			ASSERT(type==REG_DWORD);
			RegCloseKey(hKey);
			CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": Using stored value %ld for \"%s\"\n"), dwRetVal, lpMessage);
			return (UINT)dwRetVal;			//return with the last saved value
		}
		else
		{
			RegCloseKey(hKey);
		}
	}

	CMessageBox box;
	box.m_bShowCheck = TRUE;
	box.m_sRegistryValue = lpRegistry;
	if (!lpCheckMessage)
	{
#ifndef IDS_MSGBOX_DONOTSHOWAGAIN
		box.m_sCheckbox = _T("do not show again");
#else
		CString m_i18l;
		m_i18l.LoadString(IDS_MSGBOX_DONOTSHOWAGAIN);
		box.m_sCheckbox = m_i18l;
#endif
	}
	else
		box.m_sCheckbox = lpCheckMessage;
	box.m_bChecked = bChecked ? *bChecked : FALSE;
	box.m_sButton1 = lpButton1;
	box.m_sButton2 = lpButton2;
	box.m_sButton3 = lpButton3;
	box.m_hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	if (!box.m_hIcon)
		box.m_hIcon = (HICON)::LoadImage(nullptr, icon, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	else
		box.m_bDestroyIcon = TRUE;
	if (!IsWindow(hWnd))
		hWnd = nullptr;
	int result = box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, nDef);
	if (bChecked)
		*bChecked = box.m_bChecked;
	return result;
}

UINT CMessageBox::Show(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, int nDef, LPCTSTR icon, LPCTSTR lpButton1, LPCTSTR lpButton2/* = nullptr*/, LPCTSTR lpButton3/* = nullptr*/)
{
	CMessageBox box;
	box.m_sButton1 = lpButton1;
	box.m_sButton2 = lpButton2;
	box.m_sButton3 = lpButton3;
	box.m_hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	if (!box.m_hIcon)
		box.m_hIcon = (HICON)::LoadImage(nullptr, icon, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	else
		box.m_bDestroyIcon = TRUE;
	if (!IsWindow(hWnd))
		hWnd = nullptr;
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, nDef);
}

UINT CMessageBox::Show(HWND hWnd, UINT nMessage, UINT nCaption, int nDef, LPCTSTR icon, UINT nButton1, UINT nButton2, UINT nButton3)
{
	CString sButton1;
	CString sButton2;
	CString sButton3;
	CString sMessage;
	CString sCaption;
	sButton1.LoadString(nButton1);
	sButton2.LoadString(nButton2);
	sButton3.LoadString(nButton3);
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	return CMessageBox::Show(hWnd, sMessage, sCaption, nDef, icon, sButton1, sButton2, sButton3);
}


UINT CMessageBox::ShowCheck(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, LPCTSTR lpRegistry, UINT nCheckMessage, BOOL *bChecked)
{
	CString sMessage;
	CString sCaption;
	CString sCheckMsg;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	sCheckMsg.LoadString(nCheckMessage);
	return CMessageBox::ShowCheck(hWnd, sMessage, sCaption, uType, lpRegistry, sCheckMsg, bChecked);
}

UINT CMessageBox::ShowCheck(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, UINT uType, LPCTSTR lpRegistry, LPCTSTR lpCheckMessage, BOOL *bChecked)
{
	//check the registry if we have to show the box or just return with the last used return value
	//this would be the case if the user pressed "do not show again".
	DWORD dwRetVal;
	HKEY hKey;
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\TortoiseGit\\";
	path += AfxGetApp()->m_pszProfileName;
#endif
	if (lpRegistry && *lpRegistry && RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_EXECUTE, &hKey)==ERROR_SUCCESS)
	{
		int size = sizeof(dwRetVal);
		DWORD type;
		if (RegQueryValueEx(hKey, lpRegistry, nullptr, &type, (BYTE*)&dwRetVal,(LPDWORD)&size) == ERROR_SUCCESS)
		{
			ASSERT(type==REG_DWORD);
			RegCloseKey(hKey);
			CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": Using stored value %ld for \"%s\"\n"), dwRetVal, lpMessage);
			return (UINT)dwRetVal;			//return with the last saved value
		}
		else
		{
			RegCloseKey(hKey);
		}
	}

	CMessageBox box;
	box.m_bShowCheck = TRUE;
	box.m_sRegistryValue = lpRegistry;
	if (!lpCheckMessage)
	{
#ifndef IDS_MSGBOX_DONOTSHOWAGAIN
		box.m_sCheckbox = _T("do not show again");
#else
		CString m_i18l;
		m_i18l.LoadString(IDS_MSGBOX_DONOTSHOWAGAIN);
		box.m_sCheckbox = m_i18l;
#endif
	}
	else
		box.m_sCheckbox = lpCheckMessage;
	box.m_bChecked = bChecked ? *bChecked : FALSE;
	if (!IsWindow(hWnd))
		hWnd = nullptr;
	int result = box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, box.FillBoxStandard(uType));
	if (bChecked)
		*bChecked = box.m_bChecked;
	return result;
}

UINT CMessageBox::Show(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, LPCTSTR sHelpPath)
{
	CString sMessage;
	CString sCaption;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	return CMessageBox::Show(hWnd, sMessage, sCaption, uType, sHelpPath);
}

UINT CMessageBox::Show(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, UINT uType, LPCTSTR sHelpPath)
{
	CMessageBox box;

	if (!IsWindow(hWnd))
		hWnd = nullptr;
	if (sHelpPath)
		box.SetHelpPath(sHelpPath);
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, box.FillBoxStandard(uType));
}

UINT CMessageBox::Show(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, UINT nHelpID)
{
	CMessageBox box;
	CString sMessage;
	CString sCaption;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);

	if (!IsWindow(hWnd))
		hWnd = nullptr;
	box.SetHelpID(nHelpID);

	return box.GoModal(CWnd::FromHandle(hWnd), sCaption, sMessage, box.FillBoxStandard(uType));
}

bool CMessageBox::RemoveRegistryKey(LPCTSTR lpRegistry)
{
	HKEY hKey;
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\TortoiseGit\\";
	path += AfxGetApp()->m_pszProfileName;
#endif
	if (RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		bool ret = !!RegDeleteValue(hKey, lpRegistry);
		RegCloseKey(hKey);
		return ret;
	}
	return false;
}

int CMessageBox::FillBoxStandard(UINT uType)
{
	int ret = 1;
	m_uType = uType;
	m_uCancelRet = IDCANCEL;
	//load the icons according to uType
	switch (uType & 0xf0)
	{
	case MB_ICONEXCLAMATION:
		m_hIcon = (HICON)::LoadImage(nullptr, IDI_EXCLAMATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONEXCLAMATION);
		break;
	case MB_ICONASTERISK:
		m_hIcon = (HICON)::LoadImage(nullptr, IDI_ASTERISK, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONASTERISK);
		break;
	case MB_ICONQUESTION:
		m_hIcon = (HICON)::LoadImage(nullptr, IDI_QUESTION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONQUESTION);
		break;
	case MB_ICONHAND:
		m_hIcon = (HICON)::LoadImage(nullptr, IDI_HAND, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONHAND);
		break;
	}
	//set up the button texts
	switch (uType & 0xf)
	{
	case MB_ABORTRETRYIGNORE:
#ifndef IDS_MSGBOX_ABORT
		m_sButton1 = "&Abort";
#else
		m_i18l.LoadString(IDS_MSGBOX_ABORT);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDABORT;
#ifndef IDS_MSGBOX_RETRY
		m_sButton2 = "&Retry";
#else
		m_i18l.LoadString(IDS_MSGBOX_RETRY);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDRETRY;
#ifndef IDS_MSGBOX_IGNORE
		m_sButton3 = "&Ignore";
#else
		m_i18l.LoadString(IDS_MSGBOX_IGNORE);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDIGNORE;
		break;
	case MB_CANCELTRYCONTINUE:
#ifndef IDS_MSGBOX_CANCEL
		m_sButton1 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDCANCEL;
#ifndef IDS_MSGBOX_TRYAGAIN
		m_sButton2 = "&Try Again";
#else
		m_i18l.LoadString(IDS_MSGBOX_TRYAGAIN);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDTRYAGAIN;
#ifndef IDS_MSGBOX_CONTINUE
		m_sButton3 = "&Continue";
#else
		m_i18l.LoadString(IDS_MSGBOX_CONTINUE);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDCONTINUE;
		break;
	case MB_OKCANCEL:
#ifndef IDS_MSGBOX_OK
		m_sButton1 = "OK";
#else
		m_i18l.LoadString(IDS_MSGBOX_OK);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDOK;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton2 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDCANCEL;
		break;
	case MB_RETRYCANCEL:
#ifndef IDS_MSGBOX_RETRY
		m_sButton1 = "&Retry";
#else
		m_i18l.LoadString(IDS_MSGBOX_RETRY);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDRETRY;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton2 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDCANCEL;
		break;
	case MB_YESNO:
#ifndef IDS_MSGBOX_YES
		m_sButton1 = "&Yes";
#else
		m_i18l.LoadString(IDS_MSGBOX_YES);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDYES;
#ifndef IDS_MSGBOX_NO
		m_sButton2 = "&No";
#else
		m_i18l.LoadString(IDS_MSGBOX_NO);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDNO;
		break;
	case MB_YESNOCANCEL:
#ifndef IDS_MSGBOX_YES
		m_sButton1 = "&Yes";
#else
		m_i18l.LoadString(IDS_MSGBOX_YES);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDYES;
#ifndef IDS_MSGBOX_NO
		m_sButton2 = "&No";
#else
		m_i18l.LoadString(IDS_MSGBOX_NO);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDNO;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton3 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDCANCEL;
		break;
	case MB_OK:
	default:
#ifndef IDS_MSGBOX_OK
		m_sButton1 = "OK";
#else
		m_i18l.LoadString(IDS_MSGBOX_OK);
		m_sButton1 = m_i18l;
#endif
	}
	//now set the default button
	switch (uType & 0xf00)
	{
	case MB_DEFBUTTON2:
		ret = 2;
		break;
	case MB_DEFBUTTON3:
		ret = 3;
		break;
	}
	// do we need to add a help button?
	if (uType & MB_HELP)
	{
		CString sHelpText;
#ifndef IDS_MSGBOX_HELP
		sHelpText = _T("Help");
#else
		m_i18l.LoadString(IDS_MSGBOX_HELP);
		sHelpText = m_i18l;
#endif
		if (m_sButton2.IsEmpty())
		{
			m_sButton2 = sHelpText;
			m_uButton2Ret = IDHELP;
		}
		else if (m_sButton3.IsEmpty())
		{
			m_sButton3 = sHelpText;
			m_uButton3Ret = IDHELP;
		}
	}
	return ret;
}

UINT CMessageBox::GoModal(CWnd * pWnd, const CString& title, const CString& msg, int nDefaultButton)
{
	// pre Vista struct, needed for Windows XP
	struct OLD_NONCLIENTMETRICS
	{
		UINT	cbSize;
		int		iBorderWidth;
		int		iScrollWidth;
		int		iScrollHeight;
		int		iCaptionWidth;
		int		iCaptionHeight;
		LOGFONT	lfCaptionFont;
		int		iSmCaptionWidth;
		int		iSmCaptionHeight;
		LOGFONT	lfSmCaptionFont;
		int		iMenuWidth;
		int		iMenuHeight;
		LOGFONT	lfMenuFont;
		LOGFONT	lfStatusFont;
		LOGFONT	lfMessageFont;
	};
	const UINT cbProperSize = sizeof(OLD_NONCLIENTMETRICS);

	NONCLIENTMETRICS ncm = { 0 };
	ncm.cbSize = cbProperSize;
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

	memcpy(&m_LogFont, &(ncm.lfMessageFont), sizeof(LOGFONT));

	//the problem with the LOGFONT lfHeight is that it is not in pixels,
	//but the dialog template needs the height in pixels.
	//We need to convert those values first:
	CDC * pDC;
	if (pWnd)
		pDC = pWnd->GetDC();
	else
		pDC = GetDesktopWindow()->GetDC();
	HDC hdc;
	if (pDC)
		hdc = pDC->m_hDC;
	else
		hdc = ::GetDC(nullptr);
	if (!hdc)
	{
		HWND hw = pWnd ? pWnd->m_hWnd : nullptr;
		int defButton = nDefaultButton == 1 ? MB_DEFBUTTON1 : nDefaultButton == 2 ? MB_DEFBUTTON2 : nDefaultButton == 3 ? MB_DEFBUTTON3 : 0;
		return ::MessageBox(hw, msg, title, m_uType | defButton);
	}

	int pix = -MulDiv(m_LogFont.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
	CDlgTemplate dialogTemplate = CDlgTemplate(title, WS_CAPTION | DS_CENTER,
		0, 0, 0, 0, m_LogFont.lfFaceName, pix);
	dialogTemplate.AddButton(_T("Button1"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 1) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, IDC_MESSAGEBOX_BUTTON1);
	dialogTemplate.AddButton(_T("Button2"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 2) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, IDC_MESSAGEBOX_BUTTON2);
	dialogTemplate.AddButton(_T("Button3"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 3) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, IDC_MESSAGEBOX_BUTTON3);
	dialogTemplate.AddButton(_T("Checkbox"), WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 0,
		0, 0, 0, 0, IDC_MESSAGEBOX_CHECKBOX);

	m_nDefButton = nDefaultButton;
	m_sMessage = msg;
	InitModalIndirect(dialogTemplate, pWnd);

	return (UINT)DoModal();
}

void CMessageBox::SetRegistryValue(const CString& sValue, DWORD value)
{
	if (sValue.IsEmpty())
		return;

	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\TortoiseGit\\";
	path += AfxGetApp()->m_pszProfileName;
#endif
	DWORD disp;
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &disp) != ERROR_SUCCESS)
	{
		return;
	}
	RegSetValueEx(hKey, sValue, 0, REG_DWORD,(const BYTE*) &value, sizeof(value));
	RegCloseKey(hKey);
}

CSize CMessageBox::GetTextSize(const CString& str)
{
	CRect rect;
	GetWindowRect(&rect);

	CDC * pDC = GetDC();

	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(pDC);
	bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	//get the minimum size of the rectangle of the tooltip
	CSize sz = DrawHTML(&memDC, rect, str, m_LogFont, TRUE);

	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();

	ReleaseDC(pDC);

	return sz;
}

CSize CMessageBox::GetIconSize(HICON hIcon)
{
	CSize sz (0, 0);

	if (hIcon)
	{
		ICONINFO ii = { 0 };
		//get icon dimensions
		if (::GetIconInfo(hIcon, &ii))
		{
			sz.cx = (DWORD)(ii.xHotspot * 2);
			sz.cy = (DWORD)(ii.yHotspot * 2);
			//release icon mask bitmaps
			if(ii.hbmMask)
				::DeleteObject(ii.hbmMask);
			if(ii.hbmColor)
				::DeleteObject(ii.hbmColor);
		}
	}
	m_szIcon = sz;
	return sz;
}

CSize CMessageBox::GetButtonSize()
{
	CSize sz;
	int nButtons = 0;		//number of buttons - 1

	SetDlgItemText(IDC_MESSAGEBOX_BUTTON1, m_sButton1);
	SetDlgItemText(IDC_MESSAGEBOX_BUTTON2, m_sButton2);
	//GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->SendMessage(BM_SETSTYLE, BS_DEFPUSHBUTTON, 1);
	SetDlgItemText(IDC_MESSAGEBOX_BUTTON3, m_sButton3);
	SetDlgItemText(IDC_MESSAGEBOX_CHECKBOX, m_sCheckbox);

	CSize sz1 = GetTextSize(m_sButton1);
	CSize sz2 = GetTextSize(m_sButton2);
	CSize sz3 = GetTextSize(m_sButton3);

	sz1.cx += 2*MESSAGEBOX_BUTTONX;
	sz1.cy += 2*MESSAGEBOX_BUTTONY;

	if (sz2.cx)
	{
		sz2.cx += 2*MESSAGEBOX_BUTTONX;
		sz2.cy += 2*MESSAGEBOX_BUTTONY;
		nButtons++;
	}
	if (sz3.cx)
	{
		sz3.cx += 2*MESSAGEBOX_BUTTONX;
		sz3.cy += 2*MESSAGEBOX_BUTTONY;
		nButtons++;
	}

	GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->MoveWindow(0, 0, sz1.cx, sz1.cy);
	GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->MoveWindow(0, 0, sz2.cx, sz2.cy);
	GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->MoveWindow(0, 0, sz3.cx, sz3.cy);


	sz.cx = sz1.cx + sz2.cx + sz3.cx + (nButtons * MESSAGEBOX_BUTTONMARGIN);
	sz.cy = max(sz1.cy, sz2.cy);
	sz.cy = max(sz.cy, sz3.cy);
	m_szButtons = sz;
	if (m_bShowCheck)
	{
		CSize szCheck = GetTextSize(m_sCheckbox);
		szCheck.cx += 2*GetSystemMetrics(SM_CXMENUCHECK);
		szCheck.cy += 2*MESSAGEBOX_BUTTONY;
		sz.cx = max(sz.cx, szCheck.cx);
		sz.cy += szCheck.cy + MESSAGEBOX_BUTTONCHECKMARGIN;
		GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->MoveWindow(0, 0, szCheck.cx, szCheck.cy);
	}
	m_szAllButtons = sz;
	return sz;
}

BEGIN_MESSAGE_MAP(CMessageBox, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_MESSAGEBOX_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_MESSAGEBOX_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_MESSAGEBOX_BUTTON3, OnButton3)
END_MESSAGE_MAP()

void CMessageBox::OnPaint()
{
	CPaintDC dc(this); // device context for painting


	CRect rect;
	CRect drawrect;
	GetClientRect(&rect);
	GetClientRect(&drawrect);

	//create a memory device-context. This is done to help reduce
	//screen flicker, since we will paint the entire control to the
	//off screen device context first.
	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	memDC.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dc, 0,0, SRCCOPY);

	memDC.SetBkMode(TRANSPARENT);
	memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
	memDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	//OnDrawBackground();
	drawrect.DeflateRect(MESSAGEBOX_BORDERMARGINX, MESSAGEBOX_BORDERMARGINY);
	if (m_hIcon)
	{
		DrawIconEx(memDC.m_hDC, drawrect.left, drawrect.top +
			((drawrect.Height() - m_szAllButtons.cy - MESSAGEBOX_TEXTBUTTONMARGIN - m_szIcon.cy) / 2),
			m_hIcon, m_szIcon.cx, m_szIcon.cy, 0, nullptr, DI_NORMAL);

		drawrect.left += m_szIcon.cx + MESSAGEBOX_ICONMARGIN;
	}


	DrawHTML(&memDC, drawrect, m_sMessage, m_LogFont);


	//Copy the memory device context back into the original DC.
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0,0, SRCCOPY);

	//Cleanup resources.
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();


}

void CMessageBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (IsPointOverALink(point))
	{
		m_Cursor.SetCursor(IDC_HAND);
	}
	else
	{
		m_Cursor.Restore();
	}

	__super::OnMouseMove(nFlags, point);
}

void CMessageBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsPointOverALink(point))
	{
		CString url = GetLinkForPoint(point);
		ShellExecute(nullptr, _T("open"), url, nullptr, nullptr, SW_HIDE);
	}

	__super::OnLButtonUp(nFlags, point);
}

void CMessageBox::OnButton1()
{
	if (GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
	{
		m_bChecked = TRUE;
		SetRegistryValue(m_sRegistryValue, m_uButton1Ret);
	}
	else
	{
		m_bChecked = FALSE;
	}
	EndDialog(m_uButton1Ret);
}

void CMessageBox::OnButton2()
{
	if (GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
	{
		m_bChecked = TRUE;
		SetRegistryValue(m_sRegistryValue, m_uButton2Ret);
	}
	else
	{
		m_bChecked = FALSE;
	}
	if ((m_uButton2Ret == IDHELP)&&(!m_sHelpPath.IsEmpty()))
	{
		typedef HWND (WINAPI* FPHH)(HWND, LPCWSTR, UINT, DWORD);
		FPHH pHtmlHelp = nullptr; // Function pointer
		CAutoLibrary hInstHtmlHelp = AtlLoadSystemLibraryUsingFullPath(_T("HHCtrl.ocx"));
		HWND hHelp = nullptr;
		if (hInstHtmlHelp)
		{
			(FARPROC&)pHtmlHelp = GetProcAddress(hInstHtmlHelp, "HtmlHelpW");
			if (pHtmlHelp)
				hHelp = pHtmlHelp(m_hWnd, (LPCTSTR)m_sHelpPath, HH_DISPLAY_TOPIC, NULL);
		}
		if (!hHelp)
			::MessageBox(m_hWnd, _T("could not show help file"), _T("Help"), MB_ICONERROR);
	}
	else if (m_uButton2Ret == IDHELP)
	{
		OnHelp();
	}
	else
		EndDialog(m_uButton2Ret);
}

void CMessageBox::OnButton3()
{
	if (GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
	{
		m_bChecked = TRUE;
		SetRegistryValue(m_sRegistryValue, m_uButton3Ret);
	}
	else
	{
		m_bChecked = FALSE;
	}
	if ((m_uButton3Ret == IDHELP)&&(!m_sHelpPath.IsEmpty()))
	{
		typedef HWND (WINAPI* FPHH)(HWND, LPCWSTR, UINT, DWORD);
		FPHH pHtmlHelp = nullptr; // Function pointer
		CAutoLibrary hInstHtmlHelp = AtlLoadSystemLibraryUsingFullPath(_T("HHCtrl.ocx"));
		HWND hHelp = nullptr;
		if (hInstHtmlHelp)
		{
			(FARPROC&)pHtmlHelp = GetProcAddress(hInstHtmlHelp, "HtmlHelpW");
			if (pHtmlHelp)
				hHelp = pHtmlHelp(m_hWnd, (LPCTSTR)m_sHelpPath, HH_DISPLAY_TOPIC, NULL);
		}
		if (!hHelp)
			::MessageBox(m_hWnd, _T("could not show help file"), _T("Help"), MB_ICONERROR);
	}
	else if (m_uButton3Ret == IDHELP)
	{
		OnHelp();
	}
	else
		EndDialog(m_uButton3Ret);
}

void CMessageBox::OnCancel()
{
	if (m_uCancelRet == IDCANCEL)
		EndDialog(m_uCancelRet);
	//__super::OnCancel();
}

BOOL CMessageBox::OnInitDialog()
{
	__super::OnInitDialog();

	CRect rect(0, 0, 0, 0);

	//determine the required size of the message box
	CSize szText = GetTextSize(m_sMessage);
	CSize szIcon = GetIconSize(m_hIcon);
	CSize szButtons = GetButtonSize();

	CSize szIconText;
	szIconText.cx = szText.cx + szIcon.cx + ((szIcon.cx == 0) ? MESSAGEBOX_ICONMARGIN : (2*MESSAGEBOX_ICONMARGIN));
	szIconText.cy = max(szIcon.cy, szText.cy);

	rect.right = max(szButtons.cx, szIconText.cx);
	rect.right += 2*GetSystemMetrics(SM_CXBORDER);
	rect.right += 2*MESSAGEBOX_BORDERMARGINX;
	rect.bottom = szIconText.cy;
	rect.bottom += szButtons.cy;
	rect.bottom += 2*MESSAGEBOX_BORDERMARGINY + MESSAGEBOX_TEXTBUTTONMARGIN;
	rect.bottom += GetSystemMetrics(SM_CYCAPTION);
	rect.bottom += 2*GetSystemMetrics(SM_CYBORDER);

	MoveWindow(rect);
	CenterWindow();

	GetClientRect(rect);

	//now size and position the buttons as we need them
	ASSERT(!m_sButton1.IsEmpty());		//at least the first button must be there!
	if (m_sButton2.IsEmpty())
	{
		//only one button
		CRect rt;
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->GetWindowRect(rt);
		ScreenToClient(rt);
		rt.MoveToX(rect.left + ((rect.Width() - m_szButtons.cx)/2));
		rt.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->MoveWindow(rt);
		//hide the other two buttons
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->ShowWindow(SW_HIDE);
	}
	else if (m_sButton3.IsEmpty())
	{
		//two buttons
		CRect rt1;
		CRect rt2;
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->GetWindowRect(rt1);
		ScreenToClient(rt1);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->GetWindowRect(rt2);
		ScreenToClient(rt2);
		rt1.MoveToX(rect.left + ((rect.Width() - m_szButtons.cx)/2));
		rt1.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt2.MoveToX(rt1.right + MESSAGEBOX_BUTTONMARGIN);
		rt2.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->MoveWindow(rt1);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->MoveWindow(rt2);
		//hide the third button
		GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->ShowWindow(SW_HIDE);
	}
	else
	{
		//three buttons
		CRect buttonrect;
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->GetWindowRect(buttonrect);
		CRect rt1;
		CRect rt2;
		CRect rt3;
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->GetWindowRect(rt1);
		ScreenToClient(rt1);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->GetWindowRect(rt2);
		ScreenToClient(rt2);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->GetWindowRect(rt3);
		ScreenToClient(rt3);
		rt1.MoveToX(rect.left + ((rect.Width() - m_szButtons.cx)/2));
		rt1.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt2.MoveToX(rt1.right + MESSAGEBOX_BUTTONMARGIN);
		rt2.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt3.MoveToX(rt2.right + MESSAGEBOX_BUTTONMARGIN);
		rt3.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->MoveWindow(rt1);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->MoveWindow(rt2);
		GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->MoveWindow(rt3);
	}
	if (m_bShowCheck)
	{
		CRect rt;
		GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->GetWindowRect(rt);
		ScreenToClient(rt);
		rt.MoveToX(rect.left + MESSAGEBOX_BORDERMARGINX/*+ ((rect.Width() - szButtons.cx)/2)*/);
		rt.MoveToY(rect.bottom - MESSAGEBOX_BORDERMARGINY - szButtons.cy);
		GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->MoveWindow(rt);
		GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->ShowWindow(SW_SHOW);
	}
	else
		GetDlgItem(IDC_MESSAGEBOX_CHECKBOX)->ShowWindow(SW_HIDE);

	SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow();
	SetFocus(); //Just playing safe

	if (m_nDefButton == 1)
		GetDlgItem(IDC_MESSAGEBOX_BUTTON1)->SetFocus();
	if (m_nDefButton == 2)
		GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->SetFocus();
	if (m_nDefButton == 3)
		GetDlgItem(IDC_MESSAGEBOX_BUTTON3)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMessageBox::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case 'C':
		case VK_INSERT:
			{
				if (GetAsyncKeyState(VK_CONTROL)&0x8000)
				{
					CClipboardHelper clipboardHelper;
					if(clipboardHelper.Open(GetSafeHwnd()))
					{
						EmptyClipboard();
						CStringA sClipboard = CStringA(m_sMessage);
						HGLOBAL hClipboardData = CClipboardHelper::GlobalAlloc(sClipboard.GetLength()+1);
						char * pchData = (char*)GlobalLock(hClipboardData);
						if (pchData)
							strcpy_s(pchData, sClipboard.GetLength()+1, (LPCSTR)sClipboard);
						GlobalUnlock(hClipboardData);
						SetClipboardData(CF_TEXT,hClipboardData);
					}
					return TRUE;
				}
			}
			break;
		case VK_ESCAPE:
			{
				switch (m_uType & 0xf)
				{
				case MB_ABORTRETRYIGNORE:
					EndDialog(m_uButton1Ret);
					break;
				case MB_CANCELTRYCONTINUE:
					EndDialog(m_uButton1Ret);
					break;
				case MB_OKCANCEL:
					EndDialog(m_uButton2Ret);
					break;
				case MB_RETRYCANCEL:
					EndDialog(m_uButton2Ret);
					break;
				case MB_YESNO:
					EndDialog(m_uButton2Ret);
					break;
				case MB_YESNOCANCEL:
					EndDialog(m_uButton3Ret);
					break;
				}
			}
			break;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}






