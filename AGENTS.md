# Collaboration rules for this fork

- Communicate with the user in Russian.
- Before every group of edits, explain what you intend to change and why.
- Ask clarifying questions about ambiguous behavior before implementing it. Do not invent requirements or expand the requested scope.
- If an answer is required, wait for it before making dependent changes. Continue independent, explicitly requested work when possible.
- Do not replace archive icons or the File Manager Up icon until the user supplies or chooses their replacements.

# Confirmed project requirements

- This is a public fork of 7-Zip, currently based on upstream version 26.05. The user intends to publish both binaries and source on GitHub.
- Preserve all applicable license texts, copyright notices, and third-party attribution in source and distributions. Removing user help does not authorize deleting legal notices.
- Target Windows 10 and later.
- The shipped user interface must offer only Russian and English.
- The fork version is 1.0. Keep upstream version 26.05 separately for the About dialog.
- The About header is "7-Zip 1.0 fork by pa4H (c) 2026". Its GitHub button must open the owner's GitHub URL, which is still pending; keep the button disabled until supplied. The artwork source URL remains in THIRD_PARTY_NOTICES.txt.
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
