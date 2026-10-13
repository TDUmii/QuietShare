#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <memory>
#include <string>
#include <vector>

namespace quietshare
{
struct MonitorInfo
{
    HMONITOR handle = nullptr;
    std::wstring stableId;
    std::wstring name;
    RECT bounds{};
    LUID adapter{};
    bool adapterKnown = false;
    bool primary = false;
};
std::vector<MonitorInfo> EnumerateMonitors();

// This classifier examines only synthetic canary pixels, never notes or files.
// It is shared by the real WGC preflight and positive/negative regression tests.
enum class CanaryResult { Ambiguous, MarkerPresent, MarkerExcluded };
CanaryResult ClassifyCanary(const unsigned char *bgra, int width, int height,
                           unsigned stride, COLORREF publicA, COLORREF publicB);

// All methods run on the owning UI thread. Tick should be called about every
// 30 ms and before revealing a viewer. Status deliberately never includes paths.
class MirrorController
{
public:
    MirrorController();
    ~MirrorController();
    MirrorController(const MirrorController &) = delete;
    MirrorController &operator=(const MirrorController &) = delete;
    bool Verify(HWND privateWindow, const std::wstring &sourceId);
    // Only root-owned fixed-label monitor lists and the scoped fixed command
    // menu may bypass affinity checks. File-picker popup surfaces never do.
    void SetPublicPopups(const std::vector<HWND> &monitorLists);
    void SetPublicMenuOwner(HWND owner);
    bool IsVerified() const;
    void Invalidate();
    bool Start(const std::wstring &sourceId, const std::wstring &destinationId,
               bool capturePointer = true);
    void Stop();
    void Tick();
    bool Running() const;
    bool Busy() const;
    double Fps() const;
    HWND Projector() const;
    std::wstring Status(bool english = false) const;
private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};
} // namespace quietshare
