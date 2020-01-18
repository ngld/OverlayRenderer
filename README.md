# OverlayRenderer

This code is **not** production ready. Right now I'm experimenting with several approaches to render overlays.
Eventually, this might become the rendering backend for [OverlayPlugin](https://github.com/ngld/OverlayPlugin).

**WARNING**: The current code isn't very readable since I'm mostly focused on testing different approaches and I'm not exactly an experienced C++ coder. Once I've got a fairly stable approach, I'll clean up the code.

## Goals

* More control over CEF (OverlayPlugin currently uses CefSharp as a wrapper)
* Native rendering on Linux (through [Wine's wrapper DLLs](https://wiki.winehq.org/Winelib_User%27s_Guide#Building_Winelib_DLLs))
* Rendering in remote processes (probably based on ReShade)
