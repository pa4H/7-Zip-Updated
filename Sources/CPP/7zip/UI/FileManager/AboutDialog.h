// Modified for the public 7-Zip fork on 2026-10-29.
// AboutDialog.h
 
#ifndef ZIP7_INC_ABOUT_DIALOG_H
#define ZIP7_INC_ABOUT_DIALOG_H

#include "../../../Windows/Control/Dialog.h"

#include "AboutDialogRes.h"

class CAboutDialog: public NWindows::NControl::CModalDialog
{
public:
  virtual bool OnInit() Z7_override;
  virtual void OnHelp() Z7_override;
  virtual bool OnButtonClicked(unsigned buttonID, HWND buttonHWND) Z7_override;
  virtual bool OnNotify(UINT controlID, LPNMHDR header) Z7_override;
  INT_PTR Create(HWND wndParent = NULL);
};

#endif
