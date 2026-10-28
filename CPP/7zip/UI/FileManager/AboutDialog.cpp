// Modified for the public 7-Zip fork on 2026-10-27.
// About caption updated on 2026-10-28.
// AboutDialog.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../MyVersion.h"
#include "../../ForkConfig.h"

#include "../Common/LoadCodecs.h"

#include "AboutDialog.h"
#include "PropertyNameRes.h"

#include "HelpUtils.h"
#include "LangUtils.h"

#define kHomePageURL TEXT("https://www.7-zip.org/")
#define kHelpTopic "start.htm"

extern CCodecs *g_CodecsObj;

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
  SetItemText(IDT_ABOUT_VERSION, UString("7-Zip " Z7_FORK_VERSION " fork by pa4H (c) 2026"));
  SetItemText(IDT_ABOUT_DATE, UString("Based on 7-Zip " MY_VERSION));
  EnableItem(IDB_ABOUT_GITHUB, Z7_FORK_GITHUB_URL[0] != 0);
  
  NormalizePosition();
  return CModalDialog::OnInit();
}

void CAboutDialog::OnHelp()
{
  ShowHelpWindow(kHelpTopic);
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
