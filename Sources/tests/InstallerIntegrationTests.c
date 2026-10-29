/* Added 2026-10-28. GNU LGPL 2.1 or later.
   Exercise the production installer/uninstaller without elevation.
   Every registry write is redirected into a temporary HKCU subtree;
   file writes and shortcuts stay in the supplied workspace test directory. */
#include "../C/7zWindows.h"
#include <ShlObj.h>
#include <stdio.h>
#include <wchar.h>

static WCHAR g_testRoot[MAX_PATH];
static int g_allowShortcuts;
static int g_failures;
static HRESULT WINAPI TestGetShellFolder(HWND window, int folder, HANDLE token,
    DWORD flags, LPWSTR output);

#define SHGetFolderPathW TestGetShellFolder
#ifdef Z7_TEST_UNINSTALL
#include "../C/Util/7zipUninstall/7zipUninstall.c"
#else
#include "../C/Util/7zipInstall/7zipInstall.c"
#endif
#undef SHGetFolderPathW
#undef wcscpy
#undef wcscat
#undef wcslen

static void Check(int condition, const char *message)
{
  if (!condition)
  {
    printf("FAIL: %s\n", message);
    g_failures++;
  }
}

static HRESULT WINAPI TestGetShellFolder(HWND window, int folder, HANDLE token,
    DWORD flags, LPWSTR output)
{
  (void)window; (void)token; (void)flags;
  if (!g_allowShortcuts)
    return E_FAIL;
  wcscpy(output, g_testRoot);
  wcscat(output, folder == CSIDL_COMMON_PROGRAMS ? L"\\CommonStartMenu" : L"\\UserStartMenu");
  return S_OK;
}

static void SetValue(HKEY root, const WCHAR *subkey, const WCHAR *name, const WCHAR *value)
{
  HKEY key = NULL;
  LONG status = RegCreateKeyExW(root, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL);
  Check(status == ERROR_SUCCESS, "create isolated registry key");
  if (status == ERROR_SUCCESS)
  {
    Check(RegSetValueExW(key, name, 0, REG_SZ, (const BYTE *)value,
        (DWORD)((wcslen(value) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS, "write isolated registry value");
    RegCloseKey(key);
  }
}

static int HasValue(HKEY root, const WCHAR *subkey, const WCHAR *name, const WCHAR *expected)
{
  HKEY key = NULL;
  WCHAR value[MAX_PATH * 2];
  DWORD bytes = sizeof(value), type = 0;
  LONG status = RegOpenKeyExW(root, subkey, 0, KEY_READ, &key);
  if (status != ERROR_SUCCESS)
    return 0;
  status = RegQueryValueExW(key, name, NULL, &type, (BYTE *)value, &bytes);
  RegCloseKey(key);
  return status == ERROR_SUCCESS && type == REG_SZ && wcscmp(value, expected) == 0;
}

#ifdef Z7_TEST_UNINSTALL
static void SeedInstalledRegistry(void)
{
  WCHAR target[MAX_PATH * 2];
  unsigned i;
  SetValue(HKEY_CURRENT_USER, k_Reg_Software_7zip, L"Path", path);
  SetValue(HKEY_CURRENT_USER, k_Reg_Software_7zip, L"Path64", path);
  SetValue(HKEY_LOCAL_MACHINE, k_Reg_Software_7zip, L"Path", path);
  SetValue(HKEY_LOCAL_MACHINE, k_Reg_Software_7zip, L"Path64", path);
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"\"%sUninstall.exe\"", path);
  SetValue(HKEY_LOCAL_MACHINE, k_Uninstall_7zip, L"UninstallString", target);
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s7zFM.exe", path);
  SetValue(HKEY_LOCAL_MACHINE, k_AppPaths_7zFm, NULL, target);
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s7-zip.dll", path);
  SetValue(HKEY_CLASSES_ROOT, k_Reg_CLSID_7zip, NULL, L"7-Zip Shell Extension");
  SetValue(HKEY_CLASSES_ROOT, k_Reg_CLSID_7zip_Inproc, NULL, target);
  for (i = 0; i < Z7_ARRAY_SIZE(k_ShellEx_Items); i++)
  {
    CpyAscii(target, k_ShellEx_Items[i]);
    CatAscii(target, "\\7-Zip");
    SetValue(HKEY_CLASSES_ROOT, target, NULL, k_7zip_CLSID);
  }
  SetValue(HKEY_LOCAL_MACHINE, k_Shell_Approved, k_7zip_CLSID, L"7-Zip Shell Extension");
}
#endif

int wmain(int argc, WCHAR **argv)
{
  HKEY originalUser = NULL;
  HKEY container = NULL;
  HKEY roots[3] = { NULL, NULL, NULL };
  const HKEY predefined[3] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_CLASSES_ROOT };
  const WCHAR *const names[3] = { L"User", L"Machine", L"Classes" };
  WCHAR keyName[120];
  WCHAR target[MAX_PATH * 2];
  unsigned i;
  if (argc != 2 || wcslen(argv[1]) > MAX_PATH - 80)
    return 2;
  wcscpy(g_testRoot, argv[1]);
  swprintf_s(path, Z7_ARRAY_SIZE(path), L"%s\\Installed\\", g_testRoot);
  swprintf_s(keyName, Z7_ARRAY_SIZE(keyName), L"Software\\7-Zip-Updated-Installer-Test-%lu", GetCurrentProcessId());
  CoInitialize(NULL);
#ifndef Z7_TEST_UNINSTALL
  LoadSecurityDlls();
  CrcGenerateTable();
#endif
  if (RegOpenCurrentUser(KEY_ALL_ACCESS, &originalUser) != ERROR_SUCCESS ||
      RegCreateKeyExW(originalUser, keyName, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &container, NULL) != ERROR_SUCCESS)
    return 3;
  for (i = 0; i < 3; i++)
    if (RegCreateKeyExW(container, names[i], 0, NULL, 0, KEY_ALL_ACCESS, NULL, &roots[i], NULL) != ERROR_SUCCESS ||
        RegOverridePredefKey(predefined[i], roots[i]) != ERROR_SUCCESS)
    {
      Check(0, "redirect predefined registry roots");
      goto cleanup;
    }
  SetValue(HKEY_CURRENT_USER, L"Software\\7-Zip\\FM", L"TestUserSetting", L"keep");
  SetValue(HKEY_LOCAL_MACHINE, k_Shell_Approved, L"OtherExtension", L"keep");
#ifdef Z7_TEST_UNINSTALL
  SeedInstalledRegistry();
#endif
  g_SilentMode = True;
  Check(Install() == 0, "production install/uninstall succeeds");
#ifdef Z7_TEST_UNINSTALL
  Check(!HasValue(roots[0], k_Reg_Software_7zip, L"Path64", path), "remove owned installation path");
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s7-zip.dll", path);
  Check(!HasValue(roots[2], k_Reg_CLSID_7zip_Inproc, NULL, target), "unregister owned shell DLL");
  Check(!HasValue(roots[1], k_Shell_Approved, k_7zip_CLSID, L"7-Zip Shell Extension"), "remove owned shell approval");
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"\"%sUninstall.exe\"", path);
  Check(!HasValue(roots[1], k_Uninstall_7zip, L"UninstallString", target), "remove Apps and Features entry");
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s7zFM.exe", path);
  Check(!HasValue(roots[1], k_AppPaths_7zFm, NULL, target), "remove owned App Paths entry");
#else
  Check(HasValue(roots[0], k_Reg_Software_7zip, L"Path64", path), "remember installation path");
  Check(HasValue(roots[1], L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\7-Zip",
      L"DisplayVersion", L"1.0"), "register fork version in Apps and Features");
  Check(HasValue(roots[1], L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\7-Zip",
      L"DisplayName", k_7zip_with_Ver), "register fork product name");
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s7-zip.dll", path);
  Check(HasValue(roots[2], k_Reg_CLSID_7zip_Inproc, NULL, target), "register shell DLL");
  Check(HasValue(roots[1], k_Shell_Approved, k_7zip_CLSID, L"7-Zip Shell Extension"), "approve shell extension");
#endif
  for (i = 0; i < Z7_ARRAY_SIZE(k_ShellEx_Items); i++)
  {
    CpyAscii(target, k_ShellEx_Items[i]);
    CatAscii(target, "\\7-Zip");
#ifdef Z7_TEST_UNINSTALL
    Check(!HasValue(roots[2], target, NULL, k_7zip_CLSID), "remove context menu handler");
#else
    Check(HasValue(roots[2], target, NULL, k_7zip_CLSID), "register context menu handler");
#endif
  }
  Check(HasValue(roots[0], L"Software\\7-Zip\\FM", L"TestUserSetting", L"keep"), "preserve user settings");
  Check(HasValue(roots[1], k_Shell_Approved, L"OtherExtension", L"keep"), "preserve unrelated shell extension");
  /* Restore real COM registrations before exercising local shortcuts.
     This function only writes to paths supplied by TestGetShellFolder. */
  for (i = 0; i < 3; i++)
    Check(RegOverridePredefKey(predefined[i], NULL) == ERROR_SUCCESS, "restore predefined registry root");
  g_allowShortcuts = 1;
  SetShellProgramsGroup(NULL);
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s\\CommonStartMenu\\7-Zip\\7-Zip File Manager.lnk", g_testRoot);
#ifdef Z7_TEST_UNINSTALL
  Check(GetFileAttributesW(target) == INVALID_FILE_ATTRIBUTES, "remove local Start menu shortcut");
#else
  Check(GetFileAttributesW(target) != INVALID_FILE_ATTRIBUTES, "create local Start menu shortcut");
  swprintf_s(target, Z7_ARRAY_SIZE(target), L"%s\\CommonStartMenu\\7-Zip\\7-Zip Help.lnk", g_testRoot);
  Check(GetFileAttributesW(target) == INVALID_FILE_ATTRIBUTES, "remove obsolete help shortcut");
#endif
cleanup:
  for (i = 0; i < 3; i++)
  {
    RegOverridePredefKey(predefined[i], NULL);
    if (roots[i]) RegCloseKey(roots[i]);
  }
  RegCloseKey(container);
  Check(RegDeleteTreeW(originalUser, keyName) == ERROR_SUCCESS, "clean isolated registry subtree");
  RegCloseKey(originalUser);
  CoUninitialize();
  printf("Installer integration: %d failure(s)\n", g_failures);
  return g_failures ? 1 : 0;
}
