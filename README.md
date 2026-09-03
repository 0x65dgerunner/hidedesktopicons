# Hide Desktop Icons

[![Platform](https://img.shields.io/badge/platform-Windows-0078D6?logo=windows&logoColor=white)](https://www.microsoft.com/windows)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![Languages](https://img.shields.io/badge/languages-30%2B-blue)](#languages)
[![Memory](https://img.shields.io/badge/memory-~35%20MB-lightgrey)](#lightweight-by-design)

A small Windows utility that keeps your desktop clean by hiding desktop icons with a single shortcut — and bringing them back just as easily.

I built this app for myself and have relied on it for over 5 years. It has become an essential part of my workflow: a clutter-free desktop without extra effort.

---

## How it works

Hide Desktop Icons talks directly to the Windows desktop shell. When you toggle visibility, the app finds the desktop icon list (`SysListView32`) and shows or hides it — your files stay on the desktop; only the icons disappear.

```
Press hotkey  →  icons hidden
Press again   →  icons visible
```

That is it. No file moves, no registry hacks, and no changes to your actual desktop contents.

After setup, the app can **start with Windows**, sit quietly in the **system tray**, and respond to your **global shortcut** from anywhere — even when a game or full-screen app is in focus.

Need to organize icons or use the desktop normally? Hit **Pause** (or use the tray menu) to temporarily stop hiding. Resume when you are done.

---

## Features

- **One-key toggle** — assign any supported key or mouse button as a global shortcut
- **Start with Windows** — optional autostart so icons stay hidden after reboot
- **System tray** — runs in the background with a minimal tray icon (~2 MB in tray-only mode)
- **Pause anytime** — useful before gaming, presenting, or rearranging the desktop
- **30+ languages** — ready to use out of the box
- **Three runtime modes**
  - **Full interface** — settings window with a modern Fluent-style UI
  - **Tray only (lite)** — tiny executable, tray icon only
  - **Invisible (lite)** — no window, no tray icon; hotkey only
- **Lightweight** — about **35 MB** RAM in normal use; tray-only mode can sit around **2 MB**

---

## Usage

1. Run `HideDesktopIcons.exe` to open settings (first launch).
2. Set your preferred **global shortcut** and click **Save**.
3. Enable **Start with Windows** and **Minimize to tray on close** if you want it always available.
4. Choose a **runtime mode**:
   - **Full interface** — `HideDesktopIcons.exe`
   - **Tray only** — `HideDesktopIconsLite.exe` (or set runtime mode to *Tray only* and restart)
   - **Invisible** — `HideDesktopIconsLite.exe --invisible`
5. Press your shortcut to hide or show desktop icons.
6. Use **Pause** when you need icons visible for a while.

Settings are stored in `settings.json` next to the executable.

---

## Build from source

### Requirements

- Windows 10 or later
- Visual Studio 2022 (MSVC)
- Qt 6.7+ (Widgets, Svg)
- CMake 3.16+
- [Fluent-Qt](https://github.com/calvinhxx/Fluent-Qt) (cloned manually — see below)

### Fluent-Qt (third-party dependency)

The settings window uses **Fluent-Qt**, a Fluent Design component library for Qt Widgets. The source is expected at `third_party/Fluent-Qt` and is **not** shipped with this repository (it is listed in `.gitignore`).

**1. Clone this project**

```powershell
git clone https://github.com/0x65dgerunner/hidedesktopicons.git
cd hidedesktopicons
```

**2. Clone Fluent-Qt into `third_party/`**

From the project root:

```powershell
git clone https://github.com/calvinhxx/Fluent-Qt.git third_party/Fluent-Qt
```

If the `third_party` folder does not exist yet, create it first:

```powershell
mkdir third_party
git clone https://github.com/calvinhxx/Fluent-Qt.git third_party/Fluent-Qt
```

**3. (Optional) Pin a release tag**

For reproducible builds, check out a specific Fluent-Qt version before compiling:

```powershell
cd third_party/Fluent-Qt
git checkout v1.8.0
cd ../..
```

**Alternative: Git submodule**

If you prefer submodules, add Fluent-Qt once and then use `git submodule update --init --recursive` on future clones:

```powershell
git submodule add https://github.com/calvinhxx/Fluent-Qt.git third_party/Fluent-Qt
```

CMake picks up the library automatically via `add_subdirectory(third_party/Fluent-Qt)` and links `FluentQt::FluentQt` to the GUI target. Gallery, examples, and tests are disabled in this project's `CMakeLists.txt`.

### Steps

```powershell
git clone https://github.com/0x65dgerunner/hidedesktopicons.git
cd hidedesktopicons
git clone https://github.com/calvinhxx/Fluent-Qt.git third_party/Fluent-Qt

cmake -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/msvc2022_64"
cmake --build build --config Release
```

Outputs:

| Target | Output | Description |
|--------|--------|-------------|
| `HideDesktopIcons` | `build/Release/HideDesktopIcons.exe` | Full GUI + tray |
| `HideDesktopIconsLite` | `build/Release/HideDesktopIconsLite.exe` | Tray or invisible background app |

Locale files are copied automatically to the output folder during the build.

To disable the lite build:

```powershell
cmake -B build -DBUILD_LITE=OFF
```

---

## Languages

Arabic, Chinese (Simplified & Traditional), Czech, Danish, Dutch, English, Finnish, French, German, Greek, Hebrew, Hindi, Hungarian, Indonesian, Italian, Japanese, Korean, Malay, Norwegian Bokmål, Polish, Portuguese, Romanian, Russian, Spanish, Swedish, Thai, Turkish, Ukrainian, Vietnamese — and more via JSON files in [`locales/`](locales/).

---

## Project structure

```
HideDesktopIcons/
├── gui/          # Qt settings window (Fluent UI)
├── lite/         # Win32 tray / invisible host
├── src/          # Core logic (hotkeys, desktop icons, i18n, settings)
├── locales/      # Translation dictionaries
└── third_party/  # Fluent-Qt
```

---

## Why this exists

Windows does not offer a quick, persistent way to hide desktop icons without digging through settings or right-click menus. This tool does one job well: keep your wallpaper visible and your desktop calm, with almost no overhead and no daily friction.