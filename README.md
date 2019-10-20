# OverlayRenderer

This code is **not** production ready. Right now I'm experimenting with several approaches to render overlays.
Eventually, this might become the rendering backend for [OverlayPlugin](https://github.com/ngld/OverlayPlugin).

## Goals

* More control over CEF (OverlayPlugin currently uses CefSharp as a wrapper)
* Native rendering on Linux (through [Wine's wrapper DLLs](https://wiki.winehq.org/Winelib_User%27s_Guide#Building_Winelib_DLLs))
* Rendering in remote processes (probably based on ReShade)
