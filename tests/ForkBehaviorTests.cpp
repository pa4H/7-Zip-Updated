// Added 2026-10-27. GNU LGPL 2.1 or later.
// Exercises the production extraction policy and time formatter without
// changing the registry or extracting into user folders.
#include "../CPP/7zip/UI/FileManager/StdAfx.h"
#include "../CPP/7zip/UI/Common/ExtractDestination.h"
#include "../CPP/Windows/PropVariantConv.h"
#include <stdio.h>
#include <string.h>

static unsigned g_Failures;
static void Check(bool condition, const char *name)
{
  if (!condition)
  {
    printf("FAIL: %s\n", name);
    g_Failures++;
  }
}

int main()
{
  CExtractRootInfo empty;
  Check(!empty.HasSingleFolder(), "empty archive gets its own folder");
  CExtractRootInfo oneFile;
  oneFile.Add(L"readme.txt", false);
  Check(!oneFile.HasSingleFolder(), "single file gets archive folder");
  CExtractRootInfo explicitFolder;
  explicitFolder.Add(L"data", true);
  explicitFolder.Add(L"data/file.txt", false);
  Check(explicitFolder.HasSingleFolder(), "single explicit root folder");
  CExtractRootInfo implicitFolder;
  implicitFolder.Add(L"data/first.txt", false);
  implicitFolder.Add(L"data/sub/second.txt", false);
  Check(implicitFolder.HasSingleFolder(), "implicit directory records");
  CExtractRootInfo twoFolders;
  twoFolders.Add(L"a/file.txt", false);
  twoFolders.Add(L"b/file.txt", false);
  Check(!twoFolders.HasSingleFolder(), "two roots get archive folder");
  CExtractRootInfo mixed;
  mixed.Add(L"data/file.txt", false);
  mixed.Add(L"readme.txt", false);
  Check(!mixed.HasSingleFolder(), "root file plus folder");
  CExtractRootInfo relative;
  relative.Add(L".", true);
  relative.Add(L"./data/first.txt", false);
  relative.Add(L"data\\second.txt", false);
  Check(relative.HasSingleFolder(), "tar root marker and mixed separators");

  NExtract::CInfo info;
  info.LastBasePath = L"C:\\Users\\pa4H\\Downloads\\";
  Check(ExtractSuggestedPath(info, L"D:\\", L"files", true) ==
      L"C:\\Users\\pa4H\\Downloads\\files\\", "remembered base plus archive name");
  Check(ExtractSuggestedPath(info, L"D:\\", L"files", false) ==
      info.LastBasePath, "single folder uses base unchanged");
  const UString base = ExtractBasePath(L"C:\\Users\\pa4H\\Downloads\\files\\", L"files");
  Check(base == info.LastBasePath, "automatic folder is excluded from saved base");
  info.LastBasePath = base;
  Check(ExtractSuggestedPath(info, L"D:\\", L"other", true) ==
      L"C:\\Users\\pa4H\\Downloads\\other\\", "successive archives do not nest");
  Check(ExtractBasePath(L"D:\\chosen\\", L"files") == L"D:\\chosen\\",
      "manual directory is preserved");
  info.LastBasePath.Empty();
  Check(ExtractSuggestedPath(info, L"D:\\files\\", L"files", true) ==
      L"D:\\files\\files\\", "existing base named like archive is not removed");
  Check(ExtractArchiveFolderName(L"C:\\sample.tar.gz") == L"sample", "compound extension");
  Check(ExtractArchiveFolderName(L"C:\\sample.part01.rar") == L"sample", "multipart RAR");

  SYSTEMTIME time;
  memset(&time, 0, sizeof(time));
  time.wYear = 2026;
  time.wMonth = 10;
  time.wDay = 27;
  time.wHour = 9;
  time.wMinute = 5;
  time.wSecond = 7;
  FILETIME ft;
  Check(::SystemTimeToFileTime(&time, &ft) != FALSE, "test timestamp creation");
  char text[64];
  const unsigned flags = kTimestampPrintFlags_Force_UTC | kTimestampPrintFlags_DisableZ;
  Check(ConvertUtcFileTimeToString2(ft, 0, text, kTimestampPrintLevel_DMY_DAY, flags)
      && strcmp(text, "27-10-2026") == 0, "dd-mm-yyyy");
  Check(ConvertUtcFileTimeToString2(ft, 0, text, kTimestampPrintLevel_DMY_MIN, flags)
      && strcmp(text, "27-10-2026 09:05") == 0, "dd-mm-yyyy hh:mm");
  Check(ConvertUtcFileTimeToString2(ft, 0, text, kTimestampPrintLevel_SEC, flags)
      && strcmp(text, "2026-10-27 09:05:07") == 0, "original ISO timestamp unchanged");
  printf("%u failure(s)\n", g_Failures);
  return g_Failures ? 1 : 0;
}
