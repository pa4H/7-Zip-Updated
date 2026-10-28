"""Export the actual Windows toolbar Up icon for local redrawing.

Added 2026-10-28. Exporter code: GNU LGPL 2.1 or later.
The extracted Windows artwork is not relicensed by this script.
Requires Windows and Pillow; does not show or automate an application window.
"""
from pathlib import Path
import ctypes as ct
from ctypes import wintypes as wt
from PIL import Image


class ActivationContext(ct.Structure):
    _fields_ = [
        ("cbSize", wt.DWORD), ("dwFlags", wt.DWORD), ("lpSource", wt.LPCWSTR),
        ("wProcessorArchitecture", wt.WORD), ("wLangId", wt.WORD),
        ("lpAssemblyDirectory", wt.LPCWSTR), ("lpResourceName", wt.LPCWSTR),
        ("lpApplicationName", wt.LPCWSTR), ("hModule", wt.HMODULE),
    ]


class CommonControls(ct.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("dwICC", wt.DWORD)]


class ToolbarButton(ct.Structure):
    _fields_ = [
        ("iBitmap", ct.c_int), ("idCommand", ct.c_int),
        ("fsState", ct.c_ubyte), ("fsStyle", ct.c_ubyte),
        ("bReserved", ct.c_ubyte * (6 if ct.sizeof(ct.c_void_p) == 8 else 2)),
        ("dwData", ct.c_size_t), ("iString", ct.c_ssize_t),
    ]


class IconInfo(ct.Structure):
    _fields_ = [("fIcon", wt.BOOL), ("xHotspot", wt.DWORD),
                ("yHotspot", wt.DWORD), ("hbmMask", wt.HBITMAP), ("hbmColor", wt.HBITMAP)]


class Bitmap(ct.Structure):
    _fields_ = [("bmType", wt.LONG), ("bmWidth", wt.LONG), ("bmHeight", wt.LONG),
                ("bmWidthBytes", wt.LONG), ("bmPlanes", wt.WORD),
                ("bmBitsPixel", wt.WORD), ("bmBits", ct.c_void_p)]


class BitmapHeader(ct.Structure):
    _fields_ = [("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
                ("biPlanes", wt.WORD), ("biBitCount", wt.WORD),
                ("biCompression", wt.DWORD), ("biSizeImage", wt.DWORD),
                ("biXPelsPerMeter", wt.LONG), ("biYPelsPerMeter", wt.LONG),
                ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD)]


def api(dll, name, args, result):
    function = getattr(dll, name)
    function.argtypes = args
    function.restype = result
    return function


def main():
    root = Path(__file__).resolve().parents[1]
    output = root / "assets/up-reference"
    kernel = ct.WinDLL("kernel32", use_last_error=True)
    user = ct.WinDLL("user32", use_last_error=True)
    gdi = ct.WinDLL("gdi32", use_last_error=True)
    create_context = api(kernel, "CreateActCtxW", [ct.POINTER(ActivationContext)], wt.HANDLE)
    activate = api(kernel, "ActivateActCtx", [wt.HANDLE, ct.POINTER(ct.c_size_t)], wt.BOOL)
    deactivate = api(kernel, "DeactivateActCtx", [wt.DWORD, ct.c_size_t], wt.BOOL)
    release = api(kernel, "ReleaseActCtx", [wt.HANDLE], None)
    context = ActivationContext()
    context.cbSize = ct.sizeof(context)
    context.lpSource = str(root / "CPP/7zip/UI/FileManager/7zFM.exe.manifest")
    handle = create_context(ct.byref(context))
    if handle == ct.c_void_p(-1).value:
        raise ct.WinError(ct.get_last_error())
    cookie = ct.c_size_t()
    if not activate(handle, ct.byref(cookie)):
        release(handle)
        raise ct.WinError(ct.get_last_error())
    parent = toolbar = icon = None
    info = IconInfo()
    destroy_window = api(user, "DestroyWindow", [wt.HWND], wt.BOOL)
    destroy_icon = api(user, "DestroyIcon", [wt.HICON], wt.BOOL)
    delete_object = api(gdi, "DeleteObject", [wt.HGDIOBJ], wt.BOOL)
    try:
        controls = ct.WinDLL("comctl32", use_last_error=True)
        init = api(controls, "InitCommonControlsEx", [ct.POINTER(CommonControls)], wt.BOOL)
        init_info = CommonControls(ct.sizeof(CommonControls), 4)  # ICC_BAR_CLASSES
        if not init(ct.byref(init_info)):
            raise ct.WinError(ct.get_last_error())
        create_window = api(user, "CreateWindowExW", [wt.DWORD, wt.LPCWSTR, wt.LPCWSTR,
            wt.DWORD, ct.c_int, ct.c_int, ct.c_int, ct.c_int, wt.HWND, wt.HMENU,
            wt.HINSTANCE, ct.c_void_p], wt.HWND)
        parent = create_window(0, "STATIC", None, 0, 0, 0, 1, 1, None, None, None, None)
        if not parent:
            raise ct.WinError(ct.get_last_error())
        create_toolbar = api(controls, "CreateToolbarEx", [wt.HWND, wt.DWORD, wt.UINT,
            ct.c_int, wt.HINSTANCE, wt.UINT, ct.POINTER(ToolbarButton), ct.c_int,
            ct.c_int, ct.c_int, ct.c_int, ct.c_int, wt.UINT], wt.HWND)
        button = ToolbarButton()
        button.iBitmap = 8  # VIEW_PARENTFOLDER
        button.idCommand = 100
        button.fsState = 4
        toolbar = create_toolbar(parent, 0x40000000, 100, 11, ct.c_void_p(-1), 4,
            ct.byref(button), 1, 0, 0, 0, 0, ct.sizeof(button))  # IDB_VIEW_SMALL_COLOR
        if not toolbar:
            raise ct.WinError(ct.get_last_error())
        send = api(user, "SendMessageW", [wt.HWND, wt.UINT, wt.WPARAM, wt.LPARAM], wt.LPARAM)
        images = send(toolbar, 0x400 + 49, 0, 0)  # TB_GETIMAGELIST
        get_icon = api(controls, "ImageList_GetIcon", [wt.HANDLE, ct.c_int, wt.UINT], wt.HICON)
        icon = get_icon(images, 8, 1)
        if not icon:
            raise ct.WinError(ct.get_last_error())
        get_info = api(user, "GetIconInfo", [wt.HICON, ct.POINTER(IconInfo)], wt.BOOL)
        if not get_info(icon, ct.byref(info)) or not info.hbmColor:
            raise RuntimeError("Could not obtain the toolbar icon color and mask")
        bitmap = Bitmap()
        get_object = api(gdi, "GetObjectW", [wt.HGDIOBJ, ct.c_int, ct.c_void_p], ct.c_int)
        if not get_object(info.hbmColor, ct.sizeof(bitmap), ct.byref(bitmap)):
            raise ct.WinError(ct.get_last_error())
        width, height = bitmap.bmWidth, bitmap.bmHeight
        get_dc = api(user, "GetDC", [wt.HWND], wt.HDC)
        release_dc = api(user, "ReleaseDC", [wt.HWND, wt.HDC], ct.c_int)
        get_bits = api(gdi, "GetDIBits", [wt.HDC, wt.HBITMAP, wt.UINT, wt.UINT,
            ct.c_void_p, ct.POINTER(BitmapHeader), wt.UINT], ct.c_int)
        dc = get_dc(None)
        try:
            def pixels(bitmap_handle):
                header = BitmapHeader()
                header.biSize = ct.sizeof(header)
                header.biWidth, header.biHeight = width, -height
                header.biPlanes, header.biBitCount = 1, 32
                data = ct.create_string_buffer(width * height * 4)
                if get_bits(dc, bitmap_handle, 0, height, data, ct.byref(header), 0) != height:
                    raise ct.WinError(ct.get_last_error())
                return data.raw
            color, mask = pixels(info.hbmColor), pixels(info.hbmMask)
        finally:
            release_dc(None, dc)
        rgba = bytearray(color)
        has_alpha = any(color[3::4])
        for offset in range(0, len(rgba), 4):
            transparent = mask[offset] != 0
            rgba[offset + 3] = 0 if transparent else (color[offset + 3] if has_alpha else 255)
        image = Image.frombytes("RGBA", (width, height), bytes(rgba), "raw", "BGRA")
        output.mkdir(parents=True, exist_ok=True)
        image.save(output / "up-original.png")
        image.save(output / "up-original.ico", sizes=[(width, height)], bitmap_format="bmp")
        (output / "README.txt").write_text(
            f"Windows common-controls Up icon, exported 2026-10-28.\n"
            f"Original dimensions: {width} x {height} pixels.\n"
            "Source: comctl32, IDB_VIEW_SMALL_COLOR (4), VIEW_PARENTFOLDER (8).\n"
            "Uses the same v6 activation context and toolbar resource as the File Manager.\n"
            "PNG/ICO preserve the original size and transparency. No upscaling.\n"
            "Local reference for redrawing; this Windows artwork is not relicensed as LGPL.\n"
            "The application button is unchanged. This directory is excluded from source ZIPs.\n",
            encoding="utf-8")
        print(f"Exported {width}x{height}: {output}")
    finally:
        if info.hbmColor:
            delete_object(info.hbmColor)
        if info.hbmMask:
            delete_object(info.hbmMask)
        if icon:
            destroy_icon(icon)
        if toolbar:
            destroy_window(toolbar)
        if parent:
            destroy_window(parent)
        deactivate(0, cookie.value)
        release(handle)


if __name__ == "__main__":
    main()
