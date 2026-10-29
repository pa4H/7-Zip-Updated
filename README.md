# 7-Zip Updated! 1.0

A Windows 10 and later fork of **7-Zip 26.05**, maintained by **pa4H**.
The interface is available in **English and Russian**. This is a modified,
independent release, not an official release by Igor Pavlov.

[Download the installer](https://github.com/pa4H/7-Zip-Updated/releases/latest)
 · [Browse the sources](Sources/)

## Changes from 7-Zip 26.05

- Target Windows 10 and later, with x64 applications and both x64 and x86 Explorer extensions.
- Ship only English and Russian interface languages.
- Simplify the top menu to **File, Edit, View, Settings, About**.
- Open Settings directly from the top menu instead of the Tools submenu.
- Remove Favorites, Help, and graphical benchmark commands.
- Remove bundled user help and its Start menu shortcut while preserving legal notices.
- Keep **Details** as the only file display mode, including when loading older view preferences.
- Remove the folder-history command and its keyboard shortcut.
- Remove archive comment editing while retaining the ability to read existing comments.
- Remove the **Alternate streams** entry from the File Manager menus.
- Remove the question-mark help button from the Settings window title bar.
- Add **dd-mm-yyyy** and **dd-mm-yyyy hh:mm** date formats; use **dd-mm-yyyy** by default.
- Support the mouse Back and Forward buttons for folder navigation in each panel.
- Replace the folder browsing dialog with the modern Windows folder picker.
- Remember the extraction base directory separately from an automatically added archive folder.
- Show the three most recent extraction destinations as buttons below the path field, with full-path tooltips.
- Provide destination buttons both when extracting a selected archive and when extracting from an open archive.
- Automatically add a folder named after the archive unless its root contains exactly one folder. A single file still gets an archive-named folder.
- Recognize implicit archive folders, TAR root markers, and compound or multipart archive names when suggesting extraction paths.
- Avoid accumulating archive names in the remembered extraction base; cancelling the dialog leaves destination history unchanged.
- Replace the main toolbar artwork and 27 archive file-type icons.
- Replace the File Manager **Up** button with the supplied transparent 16 × 16 icon.
- Update About to show the fork name/version, pa4H credit, upstream version, artwork link, and Igor Pavlov copyright.
- Add separate About buttons for **7-zip.org** and this fork's GitHub repository.
- Use fork version **1.0** in Windows version resources while retaining upstream **26.05** separately.
- On a first installation, enable both Explorer integrations, disable the cascaded menu, enable menu icons and duplicate-root elimination, disable Zone.Id propagation, and enable only **Add to archive...** in the context-menu command list.
- Preserve previously saved context-menu settings and allow users to change the defaults later.
- Adapt the native 7-Zip installer and uninstaller for this fork, including product/version information, legal files, removal of obsolete help and translations, and preservation of unrelated user files.
- Add local build scripts, extraction/date behavior checks, and isolated installer/uninstaller integration checks.
- Keep the complete application sources, resources, licenses, and build scripts in **Sources/**.

## Installation

Download the x64 EXE installer from [Releases](https://github.com/pa4H/7-Zip-Updated/releases).
It **replaces the standard 7-Zip installation** and enables Explorer integration.
Administrator rights are required. Close 7-Zip before installation; replacing files
currently used by Explorer may require a restart.

## Build from source

Install Visual Studio 2022 or its Build Tools with **MSVC C++ x64/x86 tools** and a
**Windows SDK**. From the repository root, run:

```powershell
.\Sources\scripts\Build-Windows.ps1
```

To also build the native installer:

```powershell
.\Sources\scripts\Build-Windows.ps1 -Installer
```

Add `-Rebuild` after changing headers or compiler settings. Build outputs and
corresponding source archives are written to `Sources/build/`.
See [FORK.md](Sources/FORK.md) for further implementation and validation notes.

## Credits and licenses

Original 7-Zip: Copyright (c) 1999-2026 Igor Pavlov and the contributors identified
in the sources. Fork by pa4H (c) 2026.

- Upstream: [ip7z/7zip](https://github.com/ip7z/7zip) · [7-zip.org](https://www.7-zip.org/).
- Code licenses: [license overview](Sources/DOC/License.txt), [GNU LGPL](Sources/DOC/copying.txt), and [unRAR restrictions](Sources/DOC/unRarLicense.txt), plus notices in individual source files.
- Artwork collection: [RipplePiam/7zip-Theme](https://github.com/RipplePiam/7zip-Theme).
- Artwork authors, permissions, and terms: [THIRD_PARTY_NOTICES.txt](Sources/THIRD_PARTY_NOTICES.txt) and [license texts](Sources/licenses/). Toolbar artwork is subject to CC BY-NC-SA terms; the entire distribution must not be described as allowing unrestricted commercial use.

[Telegram: @pa4h232](https://t.me/pa4h232)
