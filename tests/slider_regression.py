"""Sample only the separately built synthetic QA slider, never desktop pixels."""
from pathlib import Path
import ctypes
from ctypes import wintypes
import json
import subprocess
import sys
import time
import uuid
sys.dont_write_bytecode=True
import lifecycle_regression as native
ROOT=native.ROOT
USER32=native.USER32
GDI32=ctypes.WinDLL("gdi32",use_last_error=True)
USER32.GetDC.argtypes=[wintypes.HWND]
USER32.GetDC.restype=wintypes.HDC
USER32.ReleaseDC.argtypes=[wintypes.HWND,wintypes.HDC]
USER32.GetClientRect.argtypes=[wintypes.HWND,ctypes.POINTER(wintypes.RECT)]
GDI32.GetPixel.argtypes=[wintypes.HDC,ctypes.c_int,ctypes.c_int]
GDI32.GetPixel.restype=wintypes.DWORD

def main():
    folder=ROOT/"artifacts/qa-slider"/uuid.uuid4().hex
    folder.mkdir(parents=True)
    process=subprocess.Popen([str(ROOT/"build/QuietShare-QA.exe"),"--qa-scrollbar",str(folder)])
    checks={};samples=[];failure=None;window=None
    def check(name,value):
        checks[name]=bool(value)
        if not value:raise AssertionError(name)
    try:
        deadline=time.monotonic()+8
        while time.monotonic()<deadline:
            windows=native.app_windows(process.pid)
            if windows and USER32.GetDlgItem(windows[0],130):window=windows[0];break
            time.sleep(.03)
        check("syntheticOwnedQaWindow",window and native.window_pid(window)==process.pid)
        native.send(window,0x111,123)
        slider=USER32.GetDlgItem(window,130);input_window=USER32.GetDlgItem(window,163)
        check("settingsControlsPresent",slider and input_window)
        check("fontRange8To200",native.send(slider,0x401)==8 and native.send(slider,0x402)==200)
        # Keep native position at the minimum. THUMBTRACK must paint its supplied
        # live value even before the common control's position catches up.
        native.send(slider,0x405,1,8)
        for value in (8,56,104,152,200):
            native.send(window,0x114,5|(value<<16),slider)
            rect=wintypes.RECT();USER32.GetClientRect(slider,ctypes.byref(rect))
            dc=USER32.GetDC(slider)
            if not dc:raise ctypes.WinError(ctypes.get_last_error())
            try:
                pixels=[GDI32.GetPixel(dc,x,rect.bottom//2) for x in range(rect.right)]
            finally:USER32.ReleaseDC(slider,dc)
            check(f"liveValue:{value}",native.control_text(input_window)==str(value))
            check(f"validOwnedPixels:{value}",all(pixel!=0xFFFFFFFF for pixel in pixels))
            samples.append(sum(pixel==0x00CC7A00 for pixel in pixels))
        check("fillFollowsEveryDragStep",all(b>a for a,b in zip(samples,samples[1:])))
        check("fullFillCoversChannel",samples[-1]>rect.right*.8)
        native.send(slider,0x405,1,24);native.send(window,0x114,8,slider)
        check("releaseReturnsToNativeValue",native.control_text(input_window)=="24")
    except Exception as error:failure=f"{type(error).__name__}: {error}"
    finally:
        if process.poll() is None:
            if window:native.send(window,0x10,timeout=6000)
            process.wait(timeout=8)
        report={"checks":checks,"passed":sum(checks.values()),"failed":sum(not value for value in checks.values()),
                "bluePixelsAtLiveSteps":samples,"failure":failure,"method":"Owned synthetic QA slider HWND; PID-bound messages; client pixels only"}
        (ROOT/"artifacts/qa-slider/slider-regression.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
        print(json.dumps(report,indent=2))
    return 1 if failure or report["failed"] else 0

if __name__=="__main__":sys.exit(main())
