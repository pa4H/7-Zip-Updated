# Original 7-Zip icons

These are unchanged reference copies from 7-Zip 26.05, upstream commit
`b88c6bc`. These reference copies remain unchanged. The application now uses
the user-supplied filetype resources under `assets/filetypes/`.

- `archives/`: all 27 archive-format `.ico` files from `CPP/7zip/Archive/Icons/`.
- The File Manager Up button has no separate icon file in this repository.
  `CPP/7zip/UI/FileManager/Panel.cpp` obtains `VIEW_PARENTFOLDER` from the
  Windows common-controls toolbar image set `IDB_VIEW_SMALL_COLOR`.
  A replacement will need to be added as a project resource after its design
  is specified by the user.
  On 2026-10-28 its current Windows appearance was exported to the local-only
  `assets/up-reference/up-original.png` and `up-original.ico`, at 16x16 pixels.

Original icons: Copyright (C) 1999-2026 Igor Pavlov.
See `../../DOC/License.txt`, `../../DOC/copying.txt`, and
`../../DOC/unRarLicense.txt` for the upstream distribution's license texts.
The icons fall under the GNU LGPL described in `DOC/License.txt`.
