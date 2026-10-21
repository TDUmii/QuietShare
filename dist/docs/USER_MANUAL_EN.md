# QuietShare User Manual

English edition - QuietShare v0.16.0

Read, prepare and present with private notes on Windows.

[PDF edition](QuietShare-User-Manual-EN.pdf) | [Vietnamese edition](USER_MANUAL_VI.md) | [Latest release](https://github.com/TDUmii/QuietShare/releases/latest)

## 1. Start here

QuietShare is a desktop companion for reading scripts, local documents and optional web pages while presenting. Its private control window is separate from the material you intend to show an audience.

Smart Mirror captures the selected public source display and renders it on a second display. It checks that QuietShare is excluded from the verified capture path before allowing private content. A successful check describes that local capture path. You still need to inspect the actual projector, television, meeting receiver or recording.

### Choose your workflow

- **Read on one display:** open a local file or prepare a script after capture verification. Smart Mirror is not required.
- **Present to a projector or monitor:** use Windows Extend with two distinct displays on the same GPU. Follow chapter 10.
- **Present to a wireless TV:** first establish a separate Windows display. Follow chapter 11.
- **Share in a meeting:** share the original public application through the meeting application's controls. Follow chapter 12.
- **Record in OBS:** choose and test the actual public capture source. Follow chapter 12.

Do not select the private QuietShare window as your public material. F8 masks private content and stops QuietShare projection. It does not end a meeting application's screen share or an OBS recording.

### Requirements

- Windows 10 version 2004, build 19041 or later, or Windows 11.
- A 64-bit Windows installation.
- Working Desktop Window Manager (DWM) and Windows Graphics Capture (WGC).
- For Smart Mirror, two distinct Extend displays identified on the same graphics adapter.
- For optional browsing, Microsoft Edge WebView2 Runtime.

Local file readers work offline and do not require the browser engine. Cross-adapter projection is not supported. Duplicate mode locks private content.

### A short first session

1. Install QuietShare or extract the complete portable package.
2. Open the application and wait for capture verification.
3. Read the protection status. If locked, address the displayed condition before opening private notes.
4. Open a harmless text file with Ctrl+O.
5. Adjust text size, weight and word wrap to suit the laptop.
6. Test F8 to learn the mask control before presenting.
7. For an audience workflow, complete its chapter and inspect the real receiver with sample content.

The projection Play control and the reader's auto-scroll Play control have different purposes. F10 starts or stops Smart Mirror. Space starts or pauses scrolling in the reading area.

## 2. Install, update and uninstall

Download from the [QuietShare release page](https://github.com/TDUmii/QuietShare/releases/latest). Use the current release assets together. Do not combine an executable from one release with libraries from another.

### Install for the current user

1. Close an older running copy of QuietShare.
2. Download **QuietShare-Setup.exe**.
3. Run the installer and choose English or Vietnamese.
4. Complete the installation for your Windows account. Administrator access is not required.
5. Launch QuietShare and confirm the version on Home.

Windows may apply its usual downloaded-application checks. Use the release from the project's repository and follow your organization's software policy.

### Use the portable package

1. Download the complete versioned Windows x64 ZIP.
2. Extract it to a normal local folder before running the program.
3. Keep these files together:
   - QuietShare.exe
   - Scintilla.dll
   - Lexilla.dll
   - pdfium.dll
   - WebView2Loader.dll
4. Keep the included documentation and license files with the package.
5. Run QuietShare.exe from the extracted folder.

Running only the EXE, or opening it directly inside a ZIP viewer, is not a complete portable installation.

### Update

1. Save modified text or code files.
2. Stop projection and close QuietShare.
3. For an installed copy, run the new installer.
4. For portable use, extract the complete new package into a separate folder and run that copy.
5. Confirm the new version on Home.
6. Recheck the actual audience output before the next private presentation.

Saved scripts are preserved by installation updates. Keep a separate copy of important source material as part of your normal backup practice.

### Uninstall

1. Save your work and close QuietShare.
2. For an installed copy, open Windows installed-app settings and uninstall QuietShare.
3. For a portable copy, remove its extracted application folder after closing it.

Uninstalling does not erase saved scripts or all local user data. Portable use also stores preferences and scripts in the Windows user profile. Removing the portable application folder does not clear that profile. Clear recent history separately in QuietShare when that is your intention.

## 3. Home and your workspace

Home provides quick access to files and project folders you have actually opened. It does not insert demonstration recent files into your history.

![QuietShare Home](quietshare-home.png)

The illustration shows the Home layout. Your version, language, recent entries and protection results can differ.

### Open recent work

1. Return to **Overview** on the sidebar.
2. Find the file or project in Recent Files.
3. Select the entry to reopen it.
4. Use the previous or next page controls when more entries are available.

Reopening requires the original local file or folder to remain available. A recent entry is a shortcut to that location, not an embedded backup.

### Pin or remove an entry

- Select the pin control beside an entry to keep it among favorites.
- Select it again to unpin it.
- Use the remove control beside an entry to delete that history entry.
- Select **Clear history** to remove all recent entries, including pins.

These actions do not delete the original files or folders. Removing history does not close an already open document.

### Navigate the sidebar

- **Overview** returns to Home.
- **Open files** returns to the active document.
- Right-click **Open files** to switch to another open file or compose a script.
- **Browser** opens the optional browsing workspace.
- **Verify WGC** runs capture verification. The projection Play or Stop control starts or stops Smart Mirror.
- **Settings** groups appearance, reading, workspace, projection, shortcuts and browser options.

Use tooltips to identify compact icon controls. For a small laptop display, hide unneeded bars through Layout or use F11 focus mode.

### Open a project folder

1. Choose **Open project** or press Ctrl+Shift+O.
2. Select a local folder in the protected picker.
3. Expand its folders in Explorer.
4. Double-click a file or select it and press Enter.
5. Use Ctrl+B to hide or reveal Explorer.

Explorer skips junctions and symbolic links. Hidden and heavy development folders are filtered by default. The setting to include them can reveal more entries when needed. Opening a project does not execute its code or install its dependencies.

## 4. Open, edit and save files

Use **Open file**, Ctrl+O or a local file drop. The protected picker provides familiar locations such as Desktop, Downloads, Documents, Pictures and drives.

### Choose and switch files

1. Open the picker with Ctrl+O.
2. Select a location or navigate to the containing folder.
3. Select the file and confirm.
4. Use document tabs or right-click **Open files** to switch documents.
5. Use Ctrl+Tab and Ctrl+Shift+Tab to move between documents on the active document page.

Up to 32 documents can be open, including the optional script. A file drop accepts up to 31 files at once. Viewing positions are retained while switching within the current session. Open-document restoration across application restarts is not promised.

### Supported local content

- **Text and code:** up to 5 MiB. Supported text includes UTF-8 and UTF-16 with a byte-order mark. An unknown extension must contain valid supported text, not binary data.
- **Images:** PNG, JPEG, BMP and the first frame of GIF. Maximum 25 MiB and 40 megapixels.
- **PDF:** up to 64 MiB and 10,000 pages. Password-protected PDFs are unsupported.
- **Paths:** local files and folders. Network paths, executables, installers, archives and arbitrary binaries are unsupported.

Use a copy converted into a supported format when the original cannot be opened. Renaming a binary file to a text extension does not convert it.

### Edit text or code

Editing is enabled by default. Images and PDFs remain read-only. Source code is displayed and edited as text, never executed.

1. Open the text or code file.
2. Make the required changes.
3. Look for the modified marker beside its document name.
4. Select **Save** or press Ctrl+S.
5. Read any save error before closing the document.

Saving preserves supported encoding and byte-order marks. Reading controls such as size, weight and wrap do not insert formatting into plain-text files.

To view files without editing, turn off **Settings > Reading > Allow file editing**. Save existing changes before changing modes.

### External changes and closing

QuietShare refuses to overwrite a file that changed outside the application.

1. Keep the current draft when you still need its changes.
2. Inspect the file changed by the other application.
3. Save or copy any draft text you need to preserve.
4. Use **Reload** only when you are ready to replace the current preview with disk contents.

Closing a modified file offers save, discard or cancel. File drafts stay in memory until you explicitly save. They are not automatically written over the script or original file.

## 5. Prepare scripts and read comfortably

The script is an optional local draft for speaking notes. It is separate from imported files and does not need to occupy a permanent document tab.

### Prepare a script

1. Right-click **Open files** on the sidebar.
2. Choose **Compose script**.
3. Type or paste the notes you need.
4. Allow the local autosave to complete.
5. Press Ctrl+Enter to switch between editing and reading.

Scripts autosave locally. The script editor accepts up to 500,000 characters. This is separate from the 5 MiB limit for imported text files.

Imported text and code files use explicit Save instead. Switching documents does not replace the saved script with the file you just opened.

### Record and transcribe a meeting

1. Open **Settings > Reading > Transcription language**.
2. Choose **Vietnamese** or **English**. The matching Windows speech recognition component must be installed.
3. Select **Record and transcribe meeting** on the sidebar.
4. QuietShare records the Windows playback mix only. It never opens a microphone and it does not start by itself.
5. Select the same control to stop. The WAV is saved under `%LOCALAPPDATA%\QuietShare\Meetings`.
6. Wait for the footer to show the transcription result. Windows Speech API processes the WAV locally.
7. Find the UTF-8 `.txt` transcript next to the WAV. The recognized text is also appended to the local Lời nhắc.

If the selected language component is not installed, the footer reports the missing language. The application does not invent a transcript or upload the audio.

### Scroll while speaking

1. Enter the script's reading mode.
2. Select a slow, medium or fast scrolling speed.
3. Press Space in the reading area, or use its Start control.
4. Press Space again to pause.
5. Pause before switching to another document or browser tab.

Auto-scroll applies to text reading. It does not turn PDF pages, scroll an image or start Smart Mirror. F8 stops scrolling as well as masking private content and stopping projection.

### Size, weight and wrapping

- Use the viewer toolbar to increase or decrease text size.
- Enter an exact size in points and press Enter to apply it.
- Text and scripts support 8-200 pt.
- Choose Regular, Semibold or Bold for text and scripts.
- Enable Word wrap to bring a long line onto the next visual line.
- Disable wrap when preserving the original line layout is more useful.

The default text/script size is 12 pt, corresponding to the initial 50% scale. Ctrl+0 restores the applicable content default. Code, image, PDF and web viewers use their own zoom controls.

Size, weight, wrap and theme changes preserve the reading anchor and selection where supported. This helps when adjusting a long script in the middle of a presentation.

### Move between notes, files and web

1. Pause auto-scroll.
2. Choose the target document from **Open files** or its tab.
3. For web, choose the Browser workspace and the intended tab.
4. Return through **Open files** to the current document.
5. Check the visible paragraph before resuming scrolling.

The browser workspace does not require returning to a permanent Script tab. Keep only the documents and web tabs you need for the session.

## 6. View documents and choose a theme

QuietShare starts content zoom at 50%. This is a content-view setting, not a request to change Windows display scaling. Choose the size that is readable on your own screen.

### Code and images

- Code has syntax-aware viewing and a wrap control.
- Code, image and PDF zoom range from 25% to 400%.
- Use the zoom buttons or Ctrl+- and Ctrl++ in the relevant viewer.
- Ctrl+0 restores the default content scale.
- An image viewer preserves the image's pixels. A theme does not recolor the picture.
- Only the first GIF frame is displayed.

When code lines are too long, enable wrap. When inspecting indentation or alignment, keep the original line layout and use horizontal navigation if needed.

### PDF reading

1. Open a supported local PDF.
2. Use Page Up and Page Down to change pages.
3. Use the overflow menu for page navigation or Fit Width.
4. Drag a blank part of the page to pan.
5. Drag over text to select it when the PDF provides a text layer.
6. Use Ctrl+F to search searchable text.

Scanned pages without a text layer cannot provide ordinary text search or copy. QuietShare renders one PDF page at a time. It does not edit PDF contents or remove passwords.

### Find text

1. Press Ctrl+F.
2. Type the term.
3. Press Enter for the next match.
4. Press Shift+Enter for the previous match.
5. Press Esc to close the search controls.

Search behavior depends on the active viewer. PDF text search requires a text layer.

### Choose among ten themes

Open **Settings > Appearance > Color theme**, or use the theme button on the viewer toolbar.

- Classic dark
- Light
- Warm paper
- Midnight blue
- Forest dark
- Graphite
- Soft mint
- Daylight blue
- High contrast black
- High contrast white

Try a light or high-contrast theme in a bright room. Try Warm paper for a softer reading background. Theme choice persists across launches. Themes change QuietShare's interface and text presentation. They do not recolor website contents, image pixels or PDF pages.

If text looks faint, increase window opacity toward 100%, then adjust size and weight. Font appearance also depends on Windows scaling, the display and graphics settings.

## 7. Settings, layout and shortcuts

Settings separates controls into **Appearance**, **Reading**, **Workspace**, **Projection**, **Shortcuts** and **Browser**. Scroll within a category when controls do not fit in a small window.

### Make the window usable

- Change interface language in Appearance.
- Enable or disable remembering the last window size. This is enabled by default.
- Pin the control window on top when you need to keep notes visible.
- Adjust opacity from 40% to 100%. This includes the controller's text. Public projection stays opaque.
- Adjust icon size from 12 px to 24 px.
- In **Workspace**, choose numbered labels independently for document tabs and Browser tabs when titles take too much space. The tab tooltip still shows the full path or address.
- Use Layout to show or hide the activity bar, tabs, toolbar, status bar and Explorer.
- Drag the Explorer divider to adjust its width.
- Use F11 to enter or leave focus mode.

For a small screen, first hide panels you do not need. Then adjust text size. Reducing opacity can reduce text readability even when the selected theme has strong contrast.

### Resource mode

The resource control cycles through Saver at 15 FPS, Balanced at 30 FPS and Performance at 60 FPS. These are projection processing limits. They do not guarantee the display will render at exactly that rate.

Start with Balanced for ordinary slides. Saver may be sufficient for mostly static material. Performance may be useful for motion when the GPU and display can sustain it. Close unused web tabs if a site consumes resources. Background suspension remains dependent on WebView2 and the page.

### Default shortcuts

| Shortcut | Action |
|---|---|
| F8 | Mask or reveal private content when protection permits. Masking stops projection and scrolling. |
| Ctrl+Alt+Q | Recall QuietShare while preserving foreground focus and masking state. |
| F9 | Verify the WGC capture path. |
| F10 | Start or stop Smart Mirror projection. |
| F11 | Enter or leave focus mode. |
| Ctrl+O | Open files. |
| Ctrl+Shift+O | Open a project folder. |
| Ctrl+B | Show or hide Explorer. |
| Ctrl+S | Save the active editable text or code file. |
| Ctrl+Tab | Switch to the next document or web tab on its active page. |
| Ctrl+Shift+Tab | Switch to the previous document or web tab. |
| Ctrl+Enter | Switch script editing and reading. |
| Space | Start or pause auto-scroll in the reading area. |
| Ctrl+F | Find in the active supported viewer. |
| Ctrl+- / Ctrl++ | Decrease or increase the active content size or zoom. |
| Ctrl+0 | Restore the active content default size or zoom. |
| Ctrl+T / Ctrl+W | Open or close a web tab in the browser context. |

F8 and Ctrl+Alt+Q are global only when Windows registration succeeds. Other shortcuts depend on focus and the active page.

To change one of the ten configurable shortcuts:

1. Open **Settings > Shortcuts**.
2. Select the binding to change.
3. Press the new key combination.
4. Read any conflict or reserved-key message.
5. Press Esc to cancel an unfinished change.

Reserved or duplicate combinations are rejected. Another application's ownership can prevent a global binding. Use the on-screen control or local fallback in that case.

## 8. Use the optional browser

The browser is disabled at each application launch. It requires Microsoft Edge WebView2 Runtime. Enable it from the Browser workspace or **Settings > Browser** when you need it.

### Open a page

1. Enable the browser.
2. Wait for its capture verification before loading user pages.
3. Enter an HTTP or HTTPS address, a domain, localhost address or search words on the local New tab page.
4. Confirm the address or search.
5. Use Back, Forward and Reload as needed.

Search words are submitted to Google. Websites you open receive normal network requests. Local document readers do not need these connections.

### Tabs, shortcuts and zoom

- Use Ctrl+T to open a tab.
- Use Ctrl+W in the browser context to close the active tab.
- Use Ctrl+Tab or Ctrl+Shift+Tab to move between tabs.
- Up to eight tabs can be open.
- Tabs share the local sign-in profile.
- Each tab maintains its own navigation history and zoom during the session.
- Up to ten saved website shortcuts persist locally.
- Open web tabs are not restored on the next launch.
- Closing the final tab disables browsing.

New tabs must pass capture verification. Background tabs receive suspension requests. A page or runtime may continue some activity despite that request.

If you enable browsing during projection, QuietShare holds the last public frame while verifying. This can last up to about five seconds. Failure or cancellation stops projection. Check the projection result before continuing.

### Paste an image into a web editor

1. Copy a raster image to the Windows clipboard.
2. Open a compatible editor on the intended website.
3. Click the editor so it receives keyboard focus.
4. Press Ctrl+V yourself.
5. Confirm the site's image preview before submitting anything.

Compatibility depends on the website's editor. Automatic clipboard reads are denied. Copying only an image URL or file reference is not the same as copying raster image data.

### Restrictions and device permissions

The embedded browser blocks native web file pickers, file drops, downloads, screen sharing, notifications, DevTools and native context menus. Separate-popup sign-in flows may not work.

Camera and microphone are off by default. If you enable a device preference, a native grant still requires:

- The visible active tab.
- A capture-verified page.
- HTTPS.
- A request matching the page's top-level origin.
- Windows permission for the device.

Website device grants are not persisted. Revoking a grant may reload the tab to stop device streams. Use the ordinary meeting application for public meeting screen sharing.

## 9. Understand protection before sharing

QuietShare distinguishes the Windows exclusion request, local WGC verification and what an audience actually receives. Treat them as separate checks throughout the presentation.

### What each state means

- **Windows accepted capture exclusion:** Windows read back the requested exclusion for the private window.
- **WGC verified:** the local verification found the expected exclusion behavior using its synthetic marker in the selected capture path.
- **Viewer or physical receiver unverified:** the application has not proved what the real audience sees. Inspect that receiver yourself.
- **Locked or verification failed:** private content is hidden because a required condition has not passed.

An accepted Windows request alone is not a general guarantee of safety. A local WGC success does not certify every meeting capture backend, capture card or OBS source.

### Prepare the public and private surfaces

1. Open the intended public slides, document or browser in a separate public application.
2. Keep the entire QuietShare control window on the source display.
3. For Smart Mirror, select a different audience display in Extend on the same GPU.
4. Run F9 and read the result.
5. Start projection with F10 when the setup permits it.
6. Inspect the receiver using harmless material.
7. Open private notes only after the required checks succeed.

The source is the display that contains QuietShare and the public application behind it. Smart Mirror sends that public source capture to the destination. Merely opening a file inside the private QuietShare reader does not turn that file into public projection material.

### What causes a safety stop

Duplicate mode, loss of exclusion, invalid display topology, a capture or rendering failure, suspend and an unsafe private surface can stop projection and lock private content.

An unexpected stop can change the title and status, flash the taskbar and play a Windows alert. It does not take focus or restore a minimized controller. Read **Settings > Projection** for the reason.

Output never resumes automatically after a safety stop. Correct the condition, verify again and explicitly start a new session. Minimizing the controller is supported while projecting. QuietShare constrains its window to the verified source work area during the active session.

QuietShare does not change the Windows display mode for you. It also cannot prevent an external camera from photographing the laptop screen.

## 10. Present over HDMI or a wired monitor

Use this workflow for a projector, television or monitor connected through HDMI, DisplayPort or a dock. Windows must first expose a usable second display.

### Connect and identify

1. Save your work and close private material.
2. Connect the display cable.
3. Select the matching input on the projector or monitor.
4. Open Windows display settings and confirm a second display is detected.
5. Press Win+P and choose **Extend**.
6. Identify which rectangle represents the laptop and which represents the audience display.

If Windows cannot detect the display, resolve its input, power, cable, port or driver detection before starting QuietShare. A successful wireless TV setup does not prove that the wired output uses the same GPU or driver path.

### Select and test in QuietShare

1. Keep the entire control window on the laptop display.
2. Open **Settings > Projection**.
3. Select the laptop as source and the projector as destination.
4. Use the role and resolution labels. Do not infer roles only from display numbers.
5. Put public sample material on the source, behind QuietShare.
6. Press F9 and wait for its result.
7. Press F10 or the projection Play control.
8. Look at the physical audience display and confirm the sample material is visible without private notes.

If Play fails, the app opens Projection settings with a code. Record that code before trying again. Private content disappearing indicates the protection gate failed. Repeatedly clicking Play does not correct the underlying condition.

### Common HDMI outcomes

- `QS-P001`: Windows has not exposed two displays for the mirror workflow.
- `QS-P003`: the application could not identify a display's adapter.
- `QS-P004`: the identified source and destination adapters differ.
- `QS-P005`: the private control window is not fully on the selected source.
- `QS-P013`: verification did not prove exclusion in the tested path.

These conditions do not prove that a cable or projector is defective. Use the exact recovery entry in chapter 13.

### End safely

1. Stop QuietShare projection with F10.
2. Stop any meeting share or recording in its own application.
3. Close or mask private material.
4. Change Windows display mode or unplug the cable only after ending the session.

## 11. Present to a wireless television

Wireless projection support depends on how Windows and its drivers expose the connection. QuietShare does not establish the wireless link or assume it is equivalent to HDMI.

### Establish the connection

1. Prepare the television for its supported wireless display mode.
2. Use the Windows wireless display connection workflow.
3. Wait for the connection to finish.
4. Open Windows display settings and confirm that the TV appears as a separate display.
5. Select **Extend**, not Duplicate.
6. Identify the laptop and TV before opening private notes.

If the receiver is available only as a remote stream rather than a separate Windows display, it is not a Smart Mirror destination through this workflow.

### Start a verified session

1. Place QuietShare entirely on the laptop display.
2. Open **Settings > Projection**.
3. Select the laptop as source and TV as destination.
4. Open harmless public sample material on the source.
5. Run F9.
6. Start with F10 if verification succeeds.
7. Inspect the TV directly.

A wireless display exposed through a different adapter can be unsupported. Read `QS-P003` or `QS-P004` when shown. A previous successful session is useful history, but the current display configuration still needs verification.

### Reconnection and performance

- After disconnection, expect projection to stop.
- Reconnect in Windows and restore Extend.
- Reselect the current pair if identifiers or display numbers changed.
- Run F9 and explicitly start again.
- Begin with Balanced or Saver when the material is mostly static.
- Network latency and receiver behavior can still affect the result.

Do not interpret a higher FPS preset as a repair for an unverified capture path. First correct the displayed condition, then evaluate motion and resource use.

## 12. Meetings, OBS and session recovery

Use the capture controls in the application that actually shares or records. QuietShare's mask and stop controls act on QuietShare. They do not operate another application's live stream.

### Record the meeting audio on this computer

QuietShare can record the Windows default playback mix while you explicitly enable the recording control. This is useful when the meeting audio, presentation video or remote participants are played through this computer.

1. Open **Settings > Reading > Transcription language** and choose **Vietnamese** or **English**.
2. Select **Record and transcribe meeting** on the sidebar.
3. Confirm the red record icon and the footer status show that recording is active.
4. Keep the meeting audio on the intended Windows playback device.
5. Use the script editor to write manual notes while the recording runs.
6. Select **Stop meeting recording** when the session ends, or close QuietShare to stop and finalize it.
7. Find the WAV file under `%LOCALAPPDATA%\QuietShare\Meetings`.
8. Wait for Windows Speech API to finish. The UTF-8 transcript is saved beside the WAV and appended to the local Lời nhắc.

The recorder captures system playback only. It does not open the microphone, start at launch or send the audio to QuietShare's projection path. A playback-device change can stop the recording and leave a finalized partial WAV file. Speech recognition runs locally. If the selected Windows language component is missing, the footer reports the missing language and no fake transcript is created.

### Meeting transcription status codes

| Code | Meaning | Action |
|---|---|---|
| `QS-R001` | WAV recording is missing | Start a new recording. |
| `QS-R002` | Windows Speech API could not start | Restart QuietShare and check Windows speech services. |
| `QS-R003` | WAV format is not supported | Record again from the default playback device. |
| `QS-R004` | Selected language recognizer is missing | Install the matching Windows speech language pack. |
| `QS-R005` | Windows could not recognize the audio | Confirm audio playback and the selected language. |
| `QS-R006` | Transcription was stopped | Start a new recording if needed. |
| `QS-R007` | Transcript could not be saved | Check local storage permission and free space. |

### Zoom, Meet or Teams on one display

1. Open the public document in a separate application window.
2. Keep QuietShare as the private reader.
3. Run F9. Single-display WGC verification is supported even though Smart Mirror needs two displays.
4. In the meeting application, select the original public application window for sharing.
5. Do not select the private QuietShare window.
6. Inspect the actual meeting receiver on another device using harmless sample content.
7. Continue only if the intended public material is shown without private notes.
8. End sharing through the meeting application's Stop sharing control.

If private material appears, stop the meeting share immediately. A local successful test is not proof that the meeting application uses the same capture method.

### Meet online while using a projector

1. Complete chapter 10 or 11 for the physical display.
2. Prefer the original public application window as the online meeting source.
3. Check both the physical receiver and a remote meeting receiver with sample content.
4. Repeat that check after changing display mode or the meeting's capture source.
5. Stop both the meeting share and QuietShare projection when finished.

The Smart Mirror output window also requests capture exclusion. Capturing the destination display may omit that output or show a blank region. Sharing the original public window avoids relying on capture of that excluded output.

### Record or stream with OBS

1. Create the OBS source for the original public application.
2. Prefer Window Capture when appropriate for that application.
3. If using Display Capture, select the intended public display and account for excluded output windows.
4. Do not choose the private QuietShare window as an OBS source.
5. Make a short sample recording without confidential material.
6. Play it back and inspect what was actually recorded.
7. Repeat after changing source, capture backend or display arrangement.
8. Stop recording or streaming in OBS when finished.

WGC verification does not certify every OBS capture backend. A capture card or external recorder can follow a different path again.

### Duplicate and recovery

Duplicate is unsupported for private presenting. QuietShare locks private content when it detects cloning. If you need identical public material, close private notes and use the public application directly.

After Duplicate to Extend, cable reconnection, sleep or a GPU reset:

1. Restore Extend in Windows.
2. Wait for all required displays to appear.
3. Move QuietShare fully onto the intended source.
4. Select the current source and destination.
5. Run F9 and read the result.
6. Explicitly start projection with F10.
7. Inspect every intended receiver again.

If Play does nothing and private content hides, read its code in **Settings > Projection**. Do not bypass the protection check. Chapter 13 gives the ordered repair steps.

## 13. Projection error catalog

Codes identify detected conditions. They are not automatic diagnoses of defective hardware. The internal reason appears alongside the stable code for technical support.

After a repair, verify again and inspect the intended receiver with harmless content. If the condition persists, keep private notes closed and preserve the diagnostic record described in chapter 14.

### QS-P001 `need-two-monitors`

**Condition:** Smart Mirror sees fewer than two displays.

1. Connect and power the audience display. Select its correct input.
2. Confirm Windows display settings show two separate displays.
3. Select Extend, then reselect source and destination in QuietShare.

Single-display reading and local WGC verification do not require Smart Mirror.

### QS-P002 `same-monitor`

**Condition:** Source and destination are the same display.

1. Identify the laptop and receiver in Windows display settings.
2. Select different displays as source and destination.
3. Keep QuietShare on the source, run F9 and then F10.

### QS-P003 `adapter-unknown`

**Condition:** QuietShare could not identify a selected display's GPU adapter.

1. Wait for the display connection to finish, then reselect the display.
2. Check the laptop's graphics and display adapter drivers. Reconnect the receiver if needed.
3. Restart QuietShare and try again. If identification still fails, report the code and connection type.

This code does not establish that the two displays use different GPUs.

### QS-P004 `different-adapters`

**Condition:** The identified source and destination belong to different GPU adapters. Cross-adapter copy is unsupported.

1. Check which GPU drives each display using the laptop or graphics vendor's display information.
2. If the hardware supports it, use a port or configuration where both displays are driven by one adapter.
3. If no supported same-adapter configuration exists, use the public application directly without private notes in this projection workflow.

Windows graphics preference does not guarantee changing the GPU physically connected to an output port.

### QS-P005 `control-location`

**Condition:** The private control window is not fully contained on the selected source display.

1. Move the entire QuietShare window onto the selected source.
2. Restore or reduce its size if it spans two displays.
3. Check the selected source and run verification again.

### QS-P006 `windows-unsupported`

**Condition:** Windows is below build 19041.

1. Check the Windows version and build.
2. Update to a supported Windows version that meets organizational requirements.
3. Reopen QuietShare and verify again.

### QS-P007 `dwm-unavailable`

**Condition:** Desktop composition is unavailable for the required capture workflow.

1. Return to a normal Windows desktop session.
2. Check the graphics driver and desktop composition availability.
3. Restart Windows if necessary, then reopen QuietShare and retry.

### QS-P008 `control-affinity`

**Condition:** The private window's capture exclusion was not accepted or retained.

1. Close and reopen QuietShare.
2. Confirm supported Windows and a working graphics driver.
3. Run F9. Keep private content closed if the exclusion still fails.

### QS-P009 `private-popup-affinity`

**Condition:** A private dialog or popup lacks the required capture exclusion.

1. Close private dialogs and menus.
2. Return to the control window and verify again.
3. If the same action reproduces it, report the popup-opening action without sending its private contents.

### QS-P010 `topology-unavailable`

**Condition:** Windows display topology could not be read.

1. Wait for connecting, docking or display-mode changes to finish.
2. Confirm Extend in Windows and reselect the current displays.
3. Check drivers or restart Windows if the condition persists.

### QS-P011 `display-cloned`

**Condition:** Windows Duplicate or display cloning was detected.

1. Close or mask private content.
2. Press Win+P and choose Extend.
3. Reselect the display pair, run F9 and explicitly start with F10.

If you must remain in Duplicate, use public material directly without private notes.

### QS-P012 `capture-frame`

**Condition:** Capture initialization, setup or frame acquisition failed.

1. Restore the source desktop after sleep, disconnect or a graphics transition.
2. Reopen QuietShare, select the source and verify again.
3. If repeated, check graphics drivers and include the recorded HRESULT in the report.

### QS-P013 `canary-failed`

**Condition:** The synthetic verification marker did not prove capture exclusion in the tested path.

1. Close overlapping private dialogs and finish display changes.
2. Keep QuietShare on the selected source and run F9 again.
3. If it still fails, keep notes locked and report the code and application version.

Windows accepting exclusion does not override this failed verification.

### QS-P014 `render-frame`

**Condition:** The D3D11 audience output could not be created or rendered correctly.

1. Confirm the destination display is still connected and available.
2. Reselect source and destination, then verify and retry.
3. Restart QuietShare and check graphics drivers if the failure persists.

### QS-P015 `display-topology`

**Condition:** The selected display or capture configuration changed.

1. Restore the intended Extend configuration.
2. Select the current source and destination rather than relying on old display numbers.
3. Run F9, start with F10 and inspect the receiver.

### QS-P016 `device-removed`

**Condition:** The capture GPU device became unavailable or reset.

1. Save work and restore the intended displays.
2. Close and reopen QuietShare, then verify again.
3. If repeated, check the graphics driver and report the HRESULT and preceding action.

### QS-P017 `projector-affinity`

**Condition:** The public output window lost its required capture-exclusion condition.

1. Stop the session and keep private material closed.
2. Restart QuietShare, restore the pair and verify.
3. Report repeated failures. Do not disable the guard to continue.

### QS-P018 `projector-closed`

**Condition:** The audience output window was closed.

1. Confirm whether you intended to close it.
2. Return to QuietShare and check the selected displays.
3. Verify and explicitly start again when ready to reopen the output.

### QS-P019 `projector-escape`

**Condition:** Escape stopped the audience output.

1. Confirm whether the key was intentionally pressed in that output.
2. Return to the control window.
3. Verify and explicitly start again when ready.

### QS-P020 `system-suspend`

**Condition:** Sleep or suspend interrupted projection.

1. Wake the laptop and receiver.
2. Wait for displays to reconnect and restore Extend.
3. Reselect the pair, verify and explicitly start a new session.

### QS-P021 `winrt-unavailable`

**Condition:** The required Windows Runtime capture support is unavailable.

1. Confirm that Windows meets the supported version requirements.
2. Restart QuietShare and, if needed, Windows.
3. Report the error if it persists on supported Windows.

### QS-P022 `privacy-unsafe`

**Condition:** A privacy guard failed without a more specific recorded reason.

1. Check Extend and the Windows capture-exclusion status.
2. Close private dialogs, keep the controller on the source and verify again.
3. Keep notes closed and report repeated failures with the application version.

### QS-P023 `environment-invalidated`

**Condition:** The capture environment is no longer verified.

1. Restore the display configuration you intend to use.
2. Reselect the source and destination and run F9.
3. If repeated, record the action immediately before invalidation and report it.

### QS-P024 `source-monitor-changed`

**Condition:** The source display disappeared or changed during control-window movement.

1. Restore the intended source connection and Extend configuration.
2. Place the whole control window on that source.
3. Reselect, verify and explicitly start again.

### QS-P000 - no failure reason

This is an unverified status, not a hardware diagnosis.

1. Choose the intended source.
2. Run F9 and wait for the result.
3. Read any specific code that follows.

### QS-P099 - unknown reason

This fallback indicates a reason without a recognized public mapping.

1. Keep private notes closed.
2. Preserve the internal reason and application version.
3. Report the record using chapter 14.

## 14. Diagnose problems and keep a session checklist

### Find the latest projection record

After a failed start, read **Settings > Projection**. From v0.14.4 onward, QuietShare opens this category when a projection start fails.

If needed, open `%LOCALAPPDATA%\QuietShare\projection-stop.json` in a text editor. This is one local last-failure record, overwritten by a later failure. An absent record does not prove that no error occurred. Older releases may record only failures during an active session.

The record contains:

- `code`: public handbook code.
- `reason`: internal technical reason.
- `version`: application version.
- UTC time: when the failure was recorded.
- `phase`: `start`, `verification` or `running`.
- `hresult`: an attached technical result when available.
- `sessionMs`: session duration, zero before projection starts.
- `resourceMode`: the selected processing mode.

`0x00000000` means no failing HRESULT was attached. It does not mean projection succeeded. The record excludes screenshots, private text, file paths, URLs and display identifiers. Diagnostic write failure never permits projection.

### Report a reproducible problem

1. Record the application version shown on Home.
2. Copy the exact code and internal reason.
3. Include phase and HRESULT when present.
4. State the Windows version and connection type, such as HDMI, wireless or dock.
5. State whether Windows detects the receiver and whether the mode is Extend or Duplicate.
6. List the actions immediately before the failure and the expected result.
7. Review the small diagnostic record or crop a screenshot to the status area before sharing.

Do not send private notes, sign-in data or personal document contents to demonstrate the problem. A code from the affected laptop is more useful than unrelated logs from another machine.

### Other common problems

- **Cannot open a file:** check chapter 4's local path, format and size limits. A password-protected PDF or unsupported encoding needs a compatible copy.
- **Cannot save:** check whether another application changed the file. Preserve the draft before reloading.
- **Browser unavailable:** install or update the official [WebView2 Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/), then disable and re-enable browsing.
- **Image paste fails:** focus a compatible website editor and use explicit Ctrl+V with raster clipboard data. Web upload pickers remain blocked.
- **Text is faint:** use 100% opacity, a suitable theme, larger text and a supported weight.
- **Shortcut fails:** check the active page, keyboard focus and global registration conflicts.

### Before each presentation

1. Save work and connect the intended receiver.
2. Confirm Extend and the correct source/destination where Smart Mirror is used.
3. Confirm the entire private window is on the source.
4. Run WGC verification.
5. Check the actual receiver or sample recording with harmless material.
6. Confirm the F8 mask control and the separate stop control for each sharing application.

After a display or capture-path change, repeat verification and receiver inspection. The software does not provide a universal guarantee against operating-system failures, driver faults, every capture method or an external camera.

Scripts and recent history use Windows DPAPI encryption for the local user. Preferences and layout stay local. Browser cookies and sign-ins use a separate local WebView2 profile. These are distinct storage mechanisms. Uninstalling preserves saved scripts.

New projection reasons should receive a stable code and ordered recovery instructions in both language editions before release. The catalog covers implemented projection conditions. It does not assign fictional codes to unrelated website or file errors.

[Release downloads](https://github.com/TDUmii/QuietShare/releases/latest) | [Documentation index](README.md) | [Privacy details](PRIVACY.md) | [Troubleshooting](TROUBLESHOOTING.md)
