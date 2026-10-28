// Modified for the public 7-Zip fork on 2026-10-27.
// CopyDialog.cpp

#include "StdAfx.h"

#include "../../../Windows/FileName.h"

#include "../../../Windows/Control/Static.h"

#include "BrowseDialog.h"
#include "CopyDialog.h"
#include "LangUtils.h"
#include "../Common/ExtractDestination.h"

using namespace NWindows;

bool CCopyDialog::OnInit()
{
  #ifdef Z7_LANG
  LangSetDlgItems(*this, NULL, 0);
  #endif
  _path.Attach(GetItem(IDC_COPY));
  SetText(Title);
  if (ExtractMode)
  {
    _extractionInfo.Load();
    Value = ExtractSuggestedPath(_extractionInfo, Value, ArchiveFolderName, AddArchiveFolder);
    Strings = _extractionInfo.Paths;
    InitExtractHistoryButtons(*this, Strings, IDB_COPY_RECENT_1);
    RECT extra = { 0, 0, 0, 54 };
    ::MapDialogRect(*this, &extra);
    RECT info;
    GetClientRectOfItem(IDT_COPY_INFO, info);
    MoveItem(IDT_COPY_INFO, info.left, info.top + extra.bottom,
        RECT_SIZE_X(info), RECT_SIZE_Y(info));
    RECT window;
    GetWindowRect(&window);
    ::SetWindowPos(*this, NULL, 0, 0, RECT_SIZE_X(window), RECT_SIZE_Y(window) + extra.bottom,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }
  else
    for (unsigned i = 0; i < 3; i++)
      ShowItem_Bool(IDB_COPY_RECENT_1 + i, false);

  NControl::CStatic staticContol;
  staticContol.Attach(GetItem(IDT_COPY));
  staticContol.SetText(Static);
  #ifdef UNDER_CE
  // we do it, since WinCE selects Value\something instead of Value !!!!
  _path.AddString(Value);
  #endif
  FOR_VECTOR (i, Strings)
    _path.AddString(Strings[i]);
  _path.SetText(Value);
  SetItemText(IDT_COPY_INFO, Info);
  NormalizeSize(true);
  return CModalDialog::OnInit();
}

bool CCopyDialog::OnSize(WPARAM /* wParam */, int xSize, int ySize)
{
  int mx, my;
  GetMargins(8, mx, my);
  int bx1, bx2, by;
  GetItemSizes(IDCANCEL, bx1, by);
  GetItemSizes(IDOK, bx2, by);
  const int y = ySize - my - by;
  const int x = xSize - mx - bx1;

  InvalidateRect(NULL);

  {
    RECT r;
    GetClientRectOfItem(IDB_COPY_SET_PATH, r);
    const int bx = RECT_SIZE_X(r);
    MoveItem(IDB_COPY_SET_PATH, xSize - mx - bx, r.top, bx, RECT_SIZE_Y(r));
    ChangeSubWindowSizeX(_path, xSize - mx - mx - bx - mx);
  }

  {
    RECT r;
    GetClientRectOfItem(IDT_COPY_INFO, r);
    NControl::CStatic staticContol;
    staticContol.Attach(GetItem(IDT_COPY_INFO));
    const int yPos = r.top;
    staticContol.Move(mx, yPos, xSize - mx * 2, y - 2 - yPos);
  }

  for (unsigned i = 0; i < 3; i++)
  {
    RECT r;
    GetClientRectOfItem(IDB_COPY_RECENT_1 + i, r);
    MoveItem(IDB_COPY_RECENT_1 + i, mx, r.top, xSize - mx * 2, RECT_SIZE_Y(r));
  }

  MoveItem(IDCANCEL, x, y, bx1, by);
  MoveItem(IDOK, x - mx - bx2, y, bx2, by);

  return false;
}

bool CCopyDialog::OnButtonClicked(unsigned buttonID, HWND buttonHWND)
{
  if (ExtractMode && buttonID >= IDB_COPY_RECENT_1 && buttonID <= IDB_COPY_RECENT_3)
  {
    const unsigned index = buttonID - IDB_COPY_RECENT_1;
    if (index < Strings.Size())
    {
      _path.SetCurSel(-1);
      _path.SetText(Strings[index]);
    }
    return true;
  }
  switch (buttonID)
  {
    case IDB_COPY_SET_PATH:
      OnButtonSetPath();
      return true;
  }
  return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
}

void CCopyDialog::OnButtonSetPath()
{
  UString currentPath;
  _path.GetText(currentPath);

  const UString title = LangString(IDS_SET_FOLDER);

  UString resultPath;
  if (!MyBrowseForFolder(*this, title, currentPath, resultPath))
    return;
  NFile::NName::NormalizeDirPathPrefix(resultPath);
  _path.SetCurSel(-1);
  _path.SetText(resultPath);
}

void CCopyDialog::OnOK()
{
  _path.GetText(Value);
  if (ExtractMode)
  {
    Value.Trim();
    NFile::NName::NormalizeDirPathPrefix(Value);
    RememberExtractDestination(_extractionInfo, Value,
        AddArchiveFolder ? ArchiveFolderName : UString());
  }
  CModalDialog::OnOK();
}
