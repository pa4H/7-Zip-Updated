// Modified for the public 7-Zip fork on 2026-10-27.
// About text and artwork hyperlink updated on 2026-10-29.
// AboutDialog.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../MyVersion.h"
#include "../../ForkConfig.h"

#include "../Common/LoadCodecs.h"

#include "AboutDialog.h"
#include <CommCtrl.h>
#include "PropertyNameRes.h"

#include "HelpUtils.h"
#include "LangUtils.h"

#define kHomePageURL TEXT("https://www.7-zip.org/")
#define kHelpTopic "start.htm"

extern CCodecs *g_CodecsObj;

INT_PTR CAboutDialog::Create(HWND wndParent)
{
  INITCOMMONCONTROLSEX controls;
  controls.dwSize = sizeof(controls);
  controls.dwICC = ICC_LINK_CLASS;
  if (!::InitCommonControlsEx(&controls))
    return -1;
  return CModalDialog::Create(IDD_ABOUT, wndParent);
}

bool CAboutDialog::OnInit()
{
  #ifdef Z7_EXTERNAL_CODECS
  if (g_CodecsObj)
  {
    UString s;
    g_CodecsObj->GetCodecsErrorMessage(s);
    if (!s.IsEmpty())
      MessageBoxW(GetParent(), s, L"7-Zip", MB_ICONERROR);
  }
  #endif

  #ifdef Z7_LANG
  LangSetWindowText(*this, IDD_ABOUT);
  #endif
  SetItemText(IDT_ABOUT_VERSION, UString("7-Zip Updated! " Z7_FORK_VERSION));
  SetItemText(IDT_ABOUT_DATE, UString("Based on 7-Zip " MY_VERSION));
  SetItemText(IDT_ABOUT_ICONS, UString(L"Icons on <a href=\"" Z7_FORK_ICONS_URL L"\">GitHub</a>"));
  EnableItem(IDB_ABOUT_GITHUB, Z7_FORK_GITHUB_URL[0] != 0);
  
  NormalizePosition();
  return CModalDialog::OnInit();
}

void CAboutDialog::OnHelp()
{
  ShowHelpWindow(kHelpTopic);
}

bool CAboutDialog::OnNotify(UINT controlID, LPNMHDR header)
{
  if (controlID == IDT_ABOUT_ICONS &&
      (header->code == NM_CLICK || header->code == NM_RETURN))
  {
    ::ShellExecuteW(NULL, L"open", Z7_FORK_ICONS_URL, NULL, NULL, SW_SHOWNORMAL);
    return true;
  }
  return CModalDialog::OnNotify(controlID, header);
}

bool CAboutDialog::OnButtonClicked(unsigned buttonID, HWND buttonHWND)
{
  LPCTSTR url;
  switch (buttonID)
  {
    case IDB_ABOUT_HOMEPAGE: url = kHomePageURL; break;
    case IDB_ABOUT_GITHUB:
      if (Z7_FORK_GITHUB_URL[0] != 0)
        ::ShellExecuteW(NULL, L"open", Z7_FORK_GITHUB_URL, NULL, NULL, SW_SHOWNORMAL);
      return true;
    default:
      return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
  }

  #ifdef UNDER_CE
  SHELLEXECUTEINFO s;
  memset(&s, 0, sizeof(s));
  s.cbSize = sizeof(s);
  s.lpFile = url;
  ::ShellExecuteEx(&s);
  #else
  ::ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
  #endif

  return true;
}
