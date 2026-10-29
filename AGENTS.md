# Collaboration rules for this fork

- Communicate with the user in Russian.
- Before every group of edits, explain what you intend to change and why.
- Ask clarifying questions about ambiguous behavior before implementing it. Do not invent requirements or expand the requested scope.
- If an answer is required, wait for it before making dependent changes. Continue independent, explicitly requested work when possible.
- Do not replace archive icons or the File Manager Up icon until the user supplies or chooses their replacements.
- As explicitly requested on 2026-10-29, do not build an installer or push/update sources on GitHub without the user's permission for that action. Routine local builds must omit -Installer.

# Confirmed project requirements

- All application sources, artwork, licenses, tests, and build scripts are in Sources/. Paths below refer to that source directory unless stated otherwise. Build from the repository root with .\Sources\scripts\Build-Windows.ps1; -Installer remains an explicit opt-in requiring user authorization. The root README is the English project description; keep its Telegram link at the bottom.

- This is a public fork of 7-Zip, currently based on upstream version 26.05. The user intends to publish both binaries and source on GitHub.
- Preserve all applicable license texts, copyright notices, and third-party attribution in source and distributions. Removing user help does not authorize deleting legal notices.
- Target Windows 10 and later.
- The shipped user interface must offer only Russian and English.
- The fork version is 1.0. Keep upstream version 26.05 separately for the About dialog.
- The About dialog shows five lines: "7-Zip Updated! 1.0", "Fork by pa4H (c) 2026", "Based on 7-Zip 26.05", "Icons on GitHub" (GitHub is a hyperlink to https://github.com/RipplePiam/7zip-Theme), and "Copyright (c) 1999-2026 Igor Pavlov". Its two bottom buttons open 7-zip.org and https://github.com/pa4H/7-Zip-Updated respectively. The artwork source also remains in THIRD_PARTY_NOTICES.txt.
- Settings and About are direct top-level menu commands. Remove Favorites, Help, benchmark, folder-history UI, comment editing, and non-table view modes. Existing archive comments remain readable.
- Add dd-mm-yyyy (default) and dd-mm-yyyy hh:mm time display formats.
- Extraction remembers its base directory separately from each automatic archive subdirectory. Add a subdirectory named after the archive except when there is exactly one root folder, including archives with implicit folder entries. A single file still gets a subdirectory.
- Show three recent extraction destinations as buttons below the path field. Use the modern Windows folder picker and support mouse Back/Forward navigation.
- The user asked for Build Tools installation instructions. Compilation and GUI validation must wait until a compiler and Windows SDK are installed; do not claim these checks passed before running them.
- Visual Studio Community 2022 with MSVC x64/x86 has now been installed. Build and run the checks before reporting a successful build.
- Toolbar artwork is supplied in assets/interface/ and approved by the user under CC BY-NC-SA. Preserve attribution, license texts, and the unchanged source theme.ini. The author's page states CC BY-NC-SA 3.0; supplied metadata states 4.0. See THIRD_PARTY_NOTICES.txt.
- On 2026-10-28 the user stated that permission for FileTypes had been obtained and confirmed publication with sources on GitHub and distribution in binaries. Use assets/filetypes/ unchanged and preserve its theme.ini and author/source attribution. Record the user-reported permission accurately; do not invent a license or claim a permission document was supplied.
- Remove the Alternate streams entry from the File Manager menus. The owner supplied assets/up-reference/up-updated.png (16x16) on 2026-10-28 and explicitly requested replacement of the Up button. Its unchanged PNG and pixel-identical ICO are in assets/interface/. Keep the extracted Windows reference in assets/up-reference/ out of source distribution.
- The native installer must replace the existing 7-Zip installation, reuse its paths and registration, and always enable Explorer integration. Display the fork name/version in setup and Apps and Features. Include x64 and x86 shell DLLs, Russian translation, and all required legal notices; remove only known obsolete upstream translations and help. Installer integration tests use a workspace directory and process-local registry overrides, without replacing the user's real installation.
- First-install context-menu defaults, confirmed on 2026-10-29: both shell integrations enabled by the installer; cascaded menu off, menu icons on, eliminate duplicate root folder on, Zone.Id propagation off, and only "Add to archive..." enabled. Apply these defaults only where settings have not been saved; preserve existing user settings and allow later changes.
