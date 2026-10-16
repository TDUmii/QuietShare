#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>

namespace quietshare
{

struct PrivacyResult
{
    bool supported = false;
    bool applied = false;
    // API readback is recorded in actualAffinity. A successful readback does
    // not verify what a meeting participant, recorder, or projector receives.
    bool verified = false;
    DWORD requestedAffinity = 0x00000011;
    DWORD actualAffinity = 0;
    DWORD error = ERROR_SUCCESS;
    std::wstring message;
};

PrivacyResult ApplyPrivacy(HWND window);
// Returns true only when the active display topology was queried. On failure
// cloned is left true so callers default to hiding private content.
bool TryGetDisplayCloned(bool &cloned);
// Conservative compatibility helper: unknown topology is treated as cloned.
bool IsDisplayCloned();
int WindowsBuild();
// Returns escaped JSON string contents, without surrounding quotation marks.
std::wstring JsonEscape(const std::wstring &text);
std::wstring GetLocalDataDirectory();
// Captures the entire virtual desktop through GDI. This is a local capture
// check, not evidence of what a particular meeting application sends.
bool CaptureDesktopBmp(const std::wstring &path);
bool CaptureRegionBmp(const std::wstring &path, const RECT &region, bool includeLayered = true);
void SetClipboardText(HWND owner, const std::wstring &text);

} // namespace quietshare
