# Vibrancy Continued — Adjustable Blur

This extension makes **Visual Studio Code genuinely transparent on Windows 11**, with an adjustable Windows backdrop effect, a lightweight glass look, and working drag-to-edge Snap behavior.

In practical terms:

- the desktop and windows behind VS Code remain visible;
- the backdrop intensity is adjustable;
- the editor, terminal and sidebars share a coherent glass appearance;
- title bars, borders and UI elements keep enough contrast to stay readable;
- transparent frameless windows can still snap left, right and maximize when dragged to the top;
- the default preset is already included — no giant `settings.json` block to copy.

The default preset currently uses:

```text
Opacity          0.18
Backdrop / Blur  3
Window type      transparent
Window mode      frameless-transparent
Window controls  custom
Theme preset     Default Dark
Terminal GPU     off
```

---

# Installation

## Requirements

You need:

- Windows 11
- x64 Visual Studio Code
- Node.js + npm
- the `code` command available in your terminal

The Windows x64 native module is already included in the repository, so you do **not** need Visual Studio Build Tools just to install and use the extension.

## Recommended source location

I recommend keeping the cloned source repository here:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur
```

This keeps the project close to your VS Code files while staying separate from extensions installed by VS Code.

> **Do not clone the repository into `.vscode\extensions`.**
>
> `.vscode\extensions` is managed by VS Code itself.
>
> Keep the source repository directly under `.vscode`, and let `npm run install:local` install the packaged extension into the proper extension directory.

Run these 4 commands in PowerShell:

```powershell
git clone https://github.com/CyrilMaz/Vibrancy_continued_adjustable_blur.git "$HOME\.vscode\Vibrancy_continued_adjustable_blur"
cd "$HOME\.vscode\Vibrancy_continued_adjustable_blur"
npm install
npm run install:local
```

Then in VS Code:

```text
Ctrl + Shift + P
```

Run:

```text
Reload Vibrancy
```

Then fully restart Visual Studio Code.

That's it.

---

# Want to modify it?

Everything important is inside the cloned repository.

With the recommended location, that means:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur
```

## Change the default blur / opacity / window mode

Edit:

```text
package.json
```

Important settings:

```text
vscode_vibrancy.opacity
vscode_vibrancy.blurRadius
vscode_vibrancy.type
vscode_vibrancy.windowMode
vscode_vibrancy.windowControlsStyle
vscode_vibrancy.disableColorCustomizations
```

Current defaults:

```json
"vscode_vibrancy.opacity": 0.18
"vscode_vibrancy.blurRadius": 3
"vscode_vibrancy.type": "transparent"
"vscode_vibrancy.windowMode": "frameless-transparent"
```

---

## Change editor / sidebar / terminal colors and opacity

Edit:

```text
themes/Default Dark.json
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\themes\Default Dark.json
```

This is where the default VS Code `workbench.colorCustomizations` preset is defined.

For example:

```json
"colorCustomizations": {
    "editorPane.background": "#1e1e1e00",
    "panel.background": "#1e1e1e00",
    "sideBar.background": "#1e1e1e2e",
    "editor.background": "#1e1e1e2e",
    "menu.background": "#1e1e1e5e"
}
```

The extension automatically writes the matching values into:

```text
workbench.colorCustomizations
```

when Vibrancy is applied.

---

## Change the glass CSS

Edit:

```text
themes/Default Dark.css
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\themes\Default Dark.css
```

This controls things such as:

```text
editor transparency
sidebar transparency
panel transparency
terminal layers
tabs
headers
title bar
activity bar
status bar
borders
```

---

## Change the Windows backdrop implementation

Edit:

```text
native/custom-blur.cc
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\native\custom-blur.cc
```

This contains the Windows Composition implementation used for the custom backdrop.

If you modify this file, you must rebuild the native module.

---

## Change transparent-window Snap behavior

Edit:

```text
runtime-pre-esm/win-transparent-snap.cjs
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\runtime-pre-esm\win-transparent-snap.cjs
```

This controls:

```text
drag left  → snap left
drag right → snap right
drag top   → maximize
```

This does not require rebuilding the C++ native module.

---

## Change runtime blur handling

Edit:

```text
runtime-pre-esm/index.cjs
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\runtime-pre-esm\index.cjs
```

This connects the Electron window to the native backdrop implementation and injects the glass CSS variables.

---

## Change automatic VS Code settings

Edit:

```text
extension/vscode-settings.js
```

Full recommended path:

```text
C:\Users\<YourName>\.vscode\Vibrancy_continued_adjustable_blur\extension\vscode-settings.js
```

This is responsible for things such as:

```text
workbench.colorCustomizations
terminal.background
terminal.integrated.gpuAcceleration
theme-related settings
```

---

# ##############################################
# For the brave who want to lose their sanity  #
# ##############################################

Here are all the details.

---

# How it works

The extension combines three different systems:

```text
Electron transparent window
        +
Windows Composition backdrop
        +
VS Code CSS / color customizations
```

They solve different parts of the problem.

---

# 1. Electron window transparency

The VS Code window is created using a transparent frameless Electron window.

The important configuration is:

```json
"vscode_vibrancy.type": "transparent",
"vscode_vibrancy.windowMode": "frameless-transparent"
```

This is what allows you to actually see windows and the desktop behind VS Code.

This is different from Mica.

Mica usually shows a desktop-derived material rather than a real live view of whatever window happens to be behind VS Code.

---

# 2. Windows backdrop

The native implementation lives in:

```text
native/custom-blur.cc
```

It uses Windows Composition with a `DesktopWindowTarget`, `SpriteVisual` and `HostBackdropBrush`.

The current `blurRadius` setting should not be interpreted as a literal Gaussian blur radius in pixels.

During development, Windows `HostBackdropBrush` was found to already introduce its own filtering.

Very small Gaussian values therefore looked nearly identical.

The current implementation instead uses the setting as a practical **backdrop intensity control**.

Default:

```json
"vscode_vibrancy.blurRadius": 3
```

A value of:

```json
"vscode_vibrancy.blurRadius": 0
```

disables the backdrop layer.

---

# 3. Glass opacity

The default global opacity is:

```json
"vscode_vibrancy.opacity": 0.18
```

That corresponds roughly to:

```text
18% dark glass
82% visible backdrop
```

The CSS runtime also creates different internal glass layers so UI areas can remain visually structured without making the whole window opaque.

---

# 4. Default Dark color preset

The preset is located in:

```text
themes/Default Dark.json
```

On a clean installation it generates approximately this:

```json
"workbench.colorCustomizations": {
    "terminal.background": "#00000000",

    "editorPane.background": "#1e1e1e00",
    "editorGroupHeader.tabsBackground": "#1e1e1e00",
    "editorGroupHeader.noTabsBackground": "#1e1e1e00",
    "breadcrumb.background": "#1e1e1e00",
    "editorGutter.background": "#1e1e1e00",
    "panel.background": "#1e1e1e00",
    "tab.activeBackground": "#1e1e1e00",
    "tab.unfocusedActiveBackground": "#1e1e1e00",

    "sideBar.background": "#1e1e1e2e",
    "sideBarTitle.background": "#1e1e1e2e",
    "activityBar.background": "#1e1e1e2e",
    "editor.background": "#1e1e1e2e",
    "tab.inactiveBackground": "#1e1e1e2e",
    "tab.unfocusedInactiveBackground": "#1e1e1e2e",

    "editorStickyScroll.background": "#1e1e1e2e",
    "editorStickyScrollGutter.background": "#1e1e1e2e",
    "sideBarStickyScroll.background": "#1e1e1e2e",
    "panelStickyScroll.background": "#1e1e1e2e",
    "terminalStickyScroll.background": "#1e1e1e2e",

    "inlineChat.background": "#1e1e1e2e",
    "editorWidget.background": "#1e1e1e2e",
    "editorHoverWidget.background": "#1e1e1e2e",
    "editorSuggestWidget.background": "#1e1e1e2e",
    "notifications.background": "#1e1e1e2e",
    "notificationCenterHeader.background": "#1e1e1e2e",

    "menu.background": "#1e1e1e5e",

    "quickInput.background": "#1e1e1e2e"
}
```

The important alpha values are:

```text
00 = fully transparent
2e = approximately 18%
5e = approximately 37%
```

---

# Why is terminal.background fully transparent?

The terminal deliberately uses:

```json
"terminal.background": "#00000000"
```

That does **not** mean the final terminal appears completely invisible.

The layers are:

```text
terminal text
      ↓
xterm background
#00000000
      ↓
VS Code panel glass layer
~18%
      ↓
Windows backdrop
      ↓
desktop / window behind VS Code
```

Keeping the xterm background transparent avoids stacking multiple dark transparent layers.

If both the terminal and panel had 18% backgrounds, the terminal would look noticeably darker than the editor.

---

# Terminal GPU acceleration

This fork currently keeps:

```json
"terminal.integrated.gpuAcceleration": "off"
```

This favors compatibility with the transparent terminal setup.

If the terminal feels slow, has graphical problems, or you need GPU/WebGL terminal features, try adding this manually to your VS Code `settings.json`:

```json
"terminal.integrated.gpuAcceleration": "auto"
```

This only affects terminal rendering.

It does not change the Windows backdrop effect.

---

# Custom Snap behavior

Transparent frameless Electron windows do not participate normally in Windows native Snap behavior.

This fork recreates the most useful actions itself.

The implementation is located in:

```text
runtime-pre-esm/win-transparent-snap.cjs
```

Current behavior:

```text
Drag to left edge
→ left half of the monitor work area

Drag to right edge
→ right half of the monitor work area

Drag to top
→ maximize
```

This preserves real transparency without restoring an opaque native window frame.

It is an emulation of the useful Snap gestures, not the full Windows Snap Layouts system.

---

# Native files

The main native files are:

```text
native/custom-blur.cc
native/vibrancy.cc
native/composition-effects.h
```

The precompiled Windows x64 binary is:

```text
native/prebuilt/vibrancy-x64.node
```

That binary is intentionally committed to the repository.

It allows Windows x64 users to install the extension without compiling native C++ code.

---

# Rebuilding the native module

You only need to do this if you modify the native files.

You will need:

```text
Visual Studio Build Tools
Desktop development with C++
Windows SDK
node-gyp
```

From the repository root:

```powershell
npx -y node-gyp@12.4.0 rebuild
```

Then copy the rebuilt binary:

```powershell
Copy-Item .\build\Release\vibrancy.node .\native\prebuilt\vibrancy-x64.node -Force
```

Then reinstall the extension:

```powershell
npm run install:local
```

Finally:

```text
Reload Vibrancy
Restart Visual Studio Code
```

---

# If you only modify JavaScript / CSS / JSON

For example:

```text
themes/Default Dark.css
themes/Default Dark.json
runtime-pre-esm/index.cjs
runtime-pre-esm/win-transparent-snap.cjs
extension/vscode-settings.js
package.json
```

you do **not** need to rebuild the C++ module.

Just run this from the repository root:

```powershell
npm run install:local
```

Then:

```text
Reload Vibrancy
Restart Visual Studio Code
```

---

# Project structure

The most useful files are:

```text
Vibrancy_continued_adjustable_blur/
│
├── package.json
│
├── extension/
│   ├── file-transforms.js
│   ├── index.js
│   └── vscode-settings.js
│
├── native/
│   ├── custom-blur.cc
│   ├── vibrancy.cc
│   ├── composition-effects.h
│   └── prebuilt/
│       └── vibrancy-x64.node
│
├── runtime-pre-esm/
│   ├── index.cjs
│   └── win-transparent-snap.cjs
│
└── themes/
    ├── Default Dark.css
    └── Default Dark.json
```

Quick reference:

| What you want to change | File |
|---|---|
| Default opacity | `package.json` |
| Default backdrop intensity | `package.json` |
| Default transparent window mode | `package.json` |
| Editor/sidebar/panel color alpha | `themes/Default Dark.json` |
| Glass CSS | `themes/Default Dark.css` |
| Windows backdrop implementation | `native/custom-blur.cc` |
| Native Node bindings | `native/vibrancy.cc` |
| Snap behavior | `runtime-pre-esm/win-transparent-snap.cjs` |
| Runtime backdrop/CSS injection | `runtime-pre-esm/index.cjs` |
| Automatic VS Code settings | `extension/vscode-settings.js` |
| Electron window patching | `extension/file-transforms.js` |

---

# Source repository vs installed extension

These are two different locations.

## Source repository

Recommended location:

```text
C:\Users\YourName\.vscode\Vibrancy_continued_adjustable_blur\
```

This contains the source code you cloned from GitHub:

```text
package.json
extension\
native\
runtime-pre-esm\
themes\
```

You can edit, rebuild and update this directory with Git.

## Installed VS Code extensions

VS Code-managed extensions are normally stored under:

```text
C:\Users\YourName\.vscode\extensions\
```

For example:

```text
C:\Users\YourName\.vscode\extensions\...
```

Do **not** manually clone this repository into that directory.

Let:

```powershell
npm run install:local
```

build the VSIX and let VS Code install it there.

In short:

```text
C:\Users\YourName\.vscode\
│
├── Vibrancy_continued_adjustable_blur\   ← source repository, managed by you / Git
│
└── extensions\                           ← installed extensions, managed by VS Code
```

---

# VS Code user settings

VS Code user settings are usually located at:

```text
%APPDATA%\Code\User\settings.json
```

which normally expands to:

```text
C:\Users\YourName\AppData\Roaming\Code\User\settings.json
```

The default preset in this fork is designed so you normally do **not** need to manually copy a large `workbench.colorCustomizations` block into this file.

The extension generates the relevant colors when Vibrancy is applied.

---

# Installed VS Code application

The installed VS Code application is usually somewhere under:

```text
%LOCALAPPDATA%\Programs\Microsoft VS Code\
```

Vibrancy modifies VS Code application files when you run:

```text
Reload Vibrancy
```

You normally should **not** edit those installed files manually.

So there are three separate locations to keep in mind:

```text
SOURCE REPOSITORY
C:\Users\YourName\.vscode\Vibrancy_continued_adjustable_blur\

INSTALLED EXTENSIONS
C:\Users\YourName\.vscode\extensions\

VS CODE USER SETTINGS
C:\Users\YourName\AppData\Roaming\Code\User\settings.json
```

And VS Code itself is normally somewhere under:

```text
C:\Users\YourName\AppData\Local\Programs\Microsoft VS Code\
```

---

# VS Code updates

VS Code updates can replace files patched by Vibrancy.

If the effect disappears after an update:

```text
Ctrl + Shift + P
→ Reload Vibrancy
→ Restart Visual Studio Code
```

Usually that is enough.

---

# Important: other transparency extensions

Do not use another extension that patches the Electron window at the same time.

In particular, avoid combining this fork with things such as:

```text
VS Code Translucent
```

Multiple extensions modifying VS Code's Electron `main.js` can produce combinations such as:

```text
Mica
Acrylic
transparent BrowserWindow
custom background colors
```

all being requested simultaneously.

Symptoms can include:

```text
fake transparency
Mica instead of live transparency
unexpected dark backgrounds
broken blur
inconsistent window behavior
```

Use one window-transparency system at a time.

---

# Troubleshooting

## No transparency

Check that Vibrancy has actually been applied:

```text
Ctrl + Shift + P
→ Reload Vibrancy
```

Then fully restart VS Code.

---

## Transparent window but no backdrop effect

Check:

```json
"vscode_vibrancy.blurRadius": 3
```

A value of:

```json
"vscode_vibrancy.blurRadius": 0
```

disables the native backdrop.

---

## The effect looks like Mica instead of real transparency

Make sure another extension is not patching the VS Code Electron window.

The intended configuration is:

```json
"vscode_vibrancy.type": "transparent",
"vscode_vibrancy.windowMode": "frameless-transparent"
```

---

## Terminal looks too dark

Make sure:

```json
"terminal.background": "#00000000"
```

is present in the generated `workbench.colorCustomizations`.

The terminal should inherit the glass appearance from the panel underneath it rather than adding another dark layer.

---

## Terminal rendering is slow

Try:

```json
"terminal.integrated.gpuAcceleration": "auto"
```

The fork defaults to:

```json
"terminal.integrated.gpuAcceleration": "off"
```

for compatibility.

---

## Snap stops working

Make sure you are using:

```json
"vscode_vibrancy.windowMode": "frameless-transparent"
```

and that Vibrancy has been reloaded.

The Snap implementation lives in:

```text
runtime-pre-esm/win-transparent-snap.cjs
```

---

# Current limitations

- Windows 11 is the primary target.
- The included native module is Windows x64.
- ARM64 requires rebuilding the native module.
- `blurRadius` is currently a backdrop-strength control rather than a precise Gaussian radius in pixels.
- Snap left/right/top is recreated by the extension rather than being native Windows Snap.
- Full Windows Snap Layouts are not implemented.
- VS Code updates may require running `Reload Vibrancy` again.
- Other Electron/window transparency extensions can conflict with this fork.

---

# Repository

```text
https://github.com/CyrilMaz/Vibrancy_continued_adjustable_blur
```

HTTPS — recommended Windows source location:

```powershell
git clone https://github.com/CyrilMaz/Vibrancy_continued_adjustable_blur.git "$HOME\.vscode\Vibrancy_continued_adjustable_blur"
```

SSH:

```powershell
git clone git@github.com:CyrilMaz/Vibrancy_continued_adjustable_blur.git "$HOME\.vscode\Vibrancy_continued_adjustable_blur"
```

---

# License / Credits

This project is a fork of `vscode-vibrancy-continued`.

Original project:

```text
https://github.com/illixion/vscode-vibrancy-continued
```

The original project license and attribution remain applicable.

This fork mainly focuses on:

```text
real Windows transparency
adjustable HostBackdrop intensity
transparent frameless window behavior
custom Snap support
a lighter configurable glass appearance
```

---

Also, don't blame me for using AI.
This project was originally made for personal use, but I decided to share it freely.
So enjoy vscode with your live wallpaper — whether you're using Lively Wallpaper, Wallpaper Engine, or anything else.

<3