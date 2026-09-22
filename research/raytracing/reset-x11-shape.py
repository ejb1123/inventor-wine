"""Diagnostic only: restore rectangular X11 bounding shape of one test window."""
import ctypes as C
import sys
x11 = C.CDLL('/nix/store/62qx8cgv8h24cq4vgh5ipyflwg1naykl-libx11-1.8.13/lib/libX11.so.6')
ext = C.CDLL('/nix/store/whs9idvicsc1zi5553xsb06795nzgi4g-libxext-1.3.7/lib/libXext.so.6')
x11.XOpenDisplay.argtypes=[C.c_char_p]; x11.XOpenDisplay.restype=C.c_void_p
x11.XSync.argtypes=[C.c_void_p,C.c_int];x11.XCloseDisplay.argtypes=[C.c_void_p]
ext.XShapeCombineMask.argtypes=[C.c_void_p,C.c_ulong,C.c_int,C.c_int,C.c_int,C.c_ulong,C.c_int]
d=x11.XOpenDisplay(None)
if not d: raise RuntimeError('No X11 display')
ext.XShapeCombineMask(d,int(sys.argv[1],0),0,0,0,0,0)
x11.XSync(d,0);x11.XCloseDisplay(d)
