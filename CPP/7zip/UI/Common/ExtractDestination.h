// Added for the public 7-Zip fork on 2026-10-27. GNU LGPL 2.1 or later.
#ifndef ZIP7_INC_EXTRACT_DESTINATION_H
#define ZIP7_INC_EXTRACT_DESTINATION_H

#include "../../../Windows/FileName.h"
#include "ZipRegistry.h"
#include "ExtractingFilePath.h"
#include <commctrl.h>

inline void InitExtractHistoryButtons(HWND owner, const UStringVector &paths, unsigned firstID)
{
  const HWND tooltips = ::CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL,
      WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, owner, NULL, ::GetModuleHandleW(NULL), NULL);
  for (unsigned i = 0; i < 3; i++)
  {
    const HWND button = ::GetDlgItem(owner, (int)(firstID + i));
    const bool visible = i < paths.Size();
    ::ShowWindow(button, visible ? SW_SHOW : SW_HIDE);
    if (!visible)
      continue;
    UString caption = paths[i];
    caption.Replace(L"&", L"&&");
    ::SetWindowTextW(button, caption);
    if (tooltips)
    {
      TOOLINFOW tool;
      memset(&tool, 0, sizeof(tool));
      tool.cbSize = sizeof(tool);
      tool.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
      tool.hwnd = owner;
      tool.uId = (UINT_PTR)button;
      tool.lpszText = (LPWSTR)(const wchar_t *)paths[i];
      ::SendMessageW(tooltips, TTM_ADDTOOLW, 0, (LPARAM)&tool);
    }
  }
}

// Only remove a suffix that this dialog automatically suggested. Keep the
// extraction base separate from the actual destinations shown in history.
inline UString ExtractBasePath(const UString &path, const UString &folderName)
{
  UString base = path;
  NWindows::NFile::NName::NormalizeDirPathPrefix(base);
  if (!folderName.IsEmpty())
  {
    UString leaf = base;
    if (!leaf.IsEmpty())
      leaf.DeleteBack();
    const int slash = leaf.ReverseFind_PathSepar();
    if (slash >= 0 && folderName.IsEqualTo_NoCase(leaf.Ptr((unsigned)slash + 1)))
      base = leaf.Left((unsigned)slash + 1);
  }
  return base;
}

inline UString ExtractSuggestedPath(const NExtract::CInfo &info,
    const UString &fallback, const UString &archiveFolder, bool addFolder)
{
  UString path = info.LastBasePath.IsEmpty() ? fallback : info.LastBasePath;
  NWindows::NFile::NName::NormalizeDirPathPrefix(path);
  if (addFolder && !archiveFolder.IsEmpty())
  {
    path += archiveFolder;
    NWindows::NFile::NName::NormalizeDirPathPrefix(path);
  }
  return path;
}

inline void RememberExtractDestination(NExtract::CInfo &info,
    const UString &path, const UString &automaticFolder)
{
  if (path.IsEmpty())
    return;
  info.LastBasePath = ExtractBasePath(path, automaticFolder);
  UString normalized = path;
  NWindows::NFile::NName::NormalizeDirPathPrefix(normalized);
  UStringVector paths;
  paths.Add(normalized);
  for (unsigned i = 0; i < info.Paths.Size() && paths.Size() < 3; i++)
  {
    UString previous = info.Paths[i];
    NWindows::NFile::NName::NormalizeDirPathPrefix(previous);
    bool duplicate = previous.IsEmpty();
    for (unsigned k = 0; k < paths.Size(); k++)
      if (previous.IsEqualTo_NoCase(paths[k]))
        duplicate = true;
    if (!duplicate)
      paths.Add(previous);
  }
  info.Paths = paths;
  info.Save();
}

// Track effective top-level entries, including formats that omit explicit
// directory records (for example a ZIP containing only folder/file.txt).
class CExtractRootInfo
{
  UString _root;
  bool _hasItems;
  bool _multiple;
  bool _rootIsFile;
public:
  CExtractRootInfo(): _hasItems(false), _multiple(false), _rootIsFile(false) {}
  void Add(const UString &itemPath, bool isDir)
  {
    UString path = itemPath;
    path.Replace(L'\\', L'/');
    while (path.IsPrefixedBy(L"./"))
      path.DeleteFrontal(2);
    if (path.IsEmpty() || (isDir && path == L"."))
      return;
    const int slash = path.Find(L'/');
    const UString root = slash < 0 ? path : path.Left((unsigned)slash);
    if (root.IsEmpty() || root == L"..")
    {
      _multiple = true;
      return;
    }
    if (_hasItems && root != _root)
      _multiple = true;
    if (!_hasItems)
      _root = root;
    _hasItems = true;
    if (slash < 0 && !isDir)
      _rootIsFile = true;
  }
  bool HasSingleFolder() const { return _hasItems && !_multiple && !_rootIsFile; }
  bool CannotBeSingleFolder() const { return _multiple || _rootIsFile; }
};

inline UString ExtractArchiveFolderName(const UString &archivePath)
{
  UString name = archivePath.Ptr((unsigned)(archivePath.ReverseFind_PathSepar() + 1));
  int dot = name.ReverseFind_Dot();
  if (dot > 0)
  {
    const UString ext = name.Ptr((unsigned)dot + 1);
    name.DeleteFrom((unsigned)dot);
    // Compound extensions and common multipart archive names.
    dot = name.ReverseFind_Dot();
    if (dot > 0)
    {
      const UString inner = name.Ptr((unsigned)dot + 1);
      if (inner.IsEqualTo_Ascii_NoCase("tar") || ext == L"001" ||
          (ext.IsEqualTo_Ascii_NoCase("rar") &&
            (inner.IsEqualTo_Ascii_NoCase("part1") ||
             inner.IsEqualTo_Ascii_NoCase("part01") ||
             inner.IsEqualTo_Ascii_NoCase("part001"))))
        name.DeleteFrom((unsigned)dot);
    }
  }
  name.Trim();
  // Archive filenames originate on disk, but avoid empty/dot-only folder names.
  if (name.IsEmpty() || name == L"." || name == L"..")
    name = "archive";
  return Get_Correct_FsFile_Name(name);
}

#endif
