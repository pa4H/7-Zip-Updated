# 7-Zip Updated! sources

This directory contains the complete source tree for **7-Zip Updated! 1.0**,
based on **7-Zip 26.05**. See the [project README](../README.md) for the full
list of changes, downloads, credits, and contact information.

Build requirements: Visual Studio 2022 or Build Tools, MSVC C++ x64/x86 tools,
and a Windows SDK. From this directory:

```powershell
.\scripts\Build-Windows.ps1
```

For a native installer, add `-Installer`. For a full rebuild, add `-Rebuild`.
Outputs are placed in `build/`. Builds do not replace the 7-Zip installed on
the build machine. Installer integration checks use a separate workspace
directory and process-local registry redirection.

The original code licenses are preserved in [DOC/](DOC/License.txt).
Artwork attribution and terms are recorded in [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).
