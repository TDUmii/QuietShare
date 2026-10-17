#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace quietshare
{
inline constexpr UINT MirrorStoppedMessage = WM_APP + 27;
struct MonitorInfo
{
    HMONITOR handle = nullptr;
    std::wstring stableId;
    std::wstring name;
    RECT bounds{};
    LUID adapter{};
    bool adapterKnown = false;
    bool primary = false;
    int displayNumber = 0; // Native \\.\DISPLAYn number, separate from the stable device ID.
};
std::vector<MonitorInfo> EnumerateMonitors();

// This classifier examines only synthetic canary pixels, never notes or files.
// It is shared by the real WGC preflight and positive/negative regression tests.
enum class CanaryResult { Ambiguous, MarkerPresent, MarkerExcluded };
CanaryResult ClassifyCanary(const unsigned char *bgra, int width, int height,
                           unsigned stride, COLORREF publicA, COLORREF publicB);

// All methods run on the owning UI thread. Tick runs before revealing a viewer
// and at TickIntervalMs while projecting. Status never includes private paths.
class MirrorController
{
public:
    MirrorController();
    ~MirrorController();
    MirrorController(const MirrorController &) = delete;
    MirrorController &operator=(const MirrorController &) = delete;
    bool Verify(HWND privateWindow, const std::wstring &sourceId,
                const std::function<bool(HWND)>& mountSurface = {});
    // A mounted surface probe freezes an existing same-source projection until
    // its positive/excluded controls and post-probe frame barrier both pass.
    // Any failed/cancelled probe still revokes and removes the public output.
    void SetResourceMode(int mode); // 0 Economy (15), 1 Balanced (30), 2 Performance (60).
    int ResourceMode() const;
    unsigned TargetFps() const;
    unsigned TickIntervalMs() const;
    // Only root-owned fixed-label monitor lists and the scoped fixed command
    // menu may bypass affinity checks. File-picker popup surfaces never do.
    void SetPublicPopups(const std::vector<HWND> &monitorLists);
    void SetPublicMenuOwner(HWND owner);
    bool IsVerified() const;
    void Invalidate();
    bool Start(const std::wstring &sourceId, const std::wstring &destinationId,
               bool capturePointer = true);
    void Stop();
    // Only an unexpected running-to-stopped transition posts a notice. A new
    // verification/start or explicit Stop invalidates any queued old notice.
    bool ConsumeStoppedNotice(unsigned serial);
    void Tick();
    bool Running() const;
    bool Busy() const;
    double Fps() const;
    HWND Projector() const;
    std::wstring Status(bool english = false) const;
private:
#if defined(QUIETSHARE_MIRROR_REGRESSION)
    friend struct MirrorRegressionAccess;
#endif
    struct Impl;
    std::shared_ptr<Impl> impl;
};
} // namespace quietshare
