# Win11DisableRoundedCorners
A simple utility that cold patches the Desktop Window Manager (uDWM.dll) in order to disable window rounded corners in Windows 11. Tested on build 22000.176.

Precompiled binaries are available in [Releases](https://github.com/valinet/Win11DisableRoundedCorners/releases).

To successfully patch and not brick your system, make sure you have only one dwm.exe process running. If you have multiple, connect normally to Windows, not using remote desktop or something similar.

Application requires active Internet connection when patching in order to download symbol files for `uDWM.dll`.

It is preferable but not mandatory to run the files from a separate directory. The app will temporarly place 2 files: `uDWM.dll` and `uDWM.pdb` in the directory they are in, overwriting any existing files.

Original `uDWM.dll` is backed up as `uDWM_win11drc.bak` in `%windir%\System32`.
