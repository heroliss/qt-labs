# Third-party notices

## Qt 6.12.0

This application dynamically links the unmodified Qt 6.12.0 Core, Gui, Widgets and Concurrent libraries and deploys Qt platform/image/style plugins from the same Qt Base distribution. Qt is copyright The Qt Company Ltd. and other contributors. Qt Base is available under LGPL-3.0-only / GPL-2.0-only / GPL-3.0-only and applicable third-party licences; this distribution uses the LGPL option for eligible Qt libraries.

- Exact source: <https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/qtbase-everywhere-src-6.12.0.tar.xz>
- Source SHA-256: `a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307`
- The Windows package includes that complete, unmodified archive at `third-party/sources/qtbase-everywhere-src-6.12.0.tar.xz`.
- Full upstream licence texts, bundled attribution files and notices are under `third-party/qt-6.12.0/`. The extraction manifest is `source.json` in that directory.
- The official binary archive and checksum are recorded by `scripts/install_qt.py` in local `.toolchain/qt-installation.json`.

Qt libraries remain separate DLLs. Users may replace them with a binary-compatible modified build; the application does not perform library signature or hash enforcement. Reverse engineering for debugging modifications to the LGPL-covered components is permitted. No Qt source modifications have been made. `qt.conf` configures relocatable runtime paths without patching Qt DLLs. Microsoft Windows fonts are loaded from the user's installed system fonts; font files are not redistributed.

## Microsoft Visual C++ runtime

The Windows package includes the redistributable x64 Microsoft Visual C++ runtime DLLs supplied with licensed Visual Studio 2026 (MSVC 14.51). They are Microsoft redistributable components, remain unmodified, and are not covered by any Qt licence. Windows system libraries are not copied into the package.

## Main application and archived code

The repository does not select a new licence for Liss's application code or its historical archive. The third-party licence texts apply to their respective components and do not automatically relicense the main project.
