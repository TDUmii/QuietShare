#pragma once
#include <windows.h>
#include <commctrl.h>
#include <array>
#include "Theme.h"
#include <string>

namespace quietshare {
enum class PreferenceCommand : unsigned {
    Mask, Recall, VerifyWgc, Projection, Focus, OpenFile, OpenFolder, Explorer, Reading, AutoScroll, Count
};
constexpr unsigned PreferenceCommandCount=static_cast<unsigned>(PreferenceCommand::Count);
struct KeyBinding { WORD key=0,modifiers=0; };
inline bool operator==(KeyBinding a,KeyBinding b){return a.key==b.key&&a.modifiers==b.modifiers;}
inline bool operator!=(KeyBinding a,KeyBinding b){return !(a==b);}
struct Preferences {
    std::array<KeyBinding,PreferenceCommandCount> shortcuts{};
    int opacityPercent=100,textWeight=400,iconSize=16,themeId=0;
    int transcriptionLanguage=0; // 0 = Vietnamese, 1 = English.
    bool browserMuted=false,browserAllowCamera=false,browserAllowMicrophone=false;
    bool rememberWindowSize=true,allowFileEditing=true;
    bool numberedDocumentTabs=false,numberedBrowserTabs=false;
    KeyBinding& Key(PreferenceCommand command){return shortcuts[static_cast<unsigned>(command)];}
    const KeyBinding& Key(PreferenceCommand command)const{return shortcuts[static_cast<unsigned>(command)];}
};
inline Preferences DefaultPreferences()
{
    Preferences p;
    p.shortcuts={{{VK_F8,0},{L'Q',MOD_CONTROL|MOD_ALT},{VK_F9,0},{VK_F10,0},{VK_F11,0},
                  {L'O',MOD_CONTROL},{L'O',MOD_CONTROL|MOD_SHIFT},{L'B',MOD_CONTROL},
                  {VK_RETURN,MOD_CONTROL},{VK_SPACE,0}}};
    return p;
}
inline bool MatchShortcut(KeyBinding binding,UINT key,UINT modifiers)
{
    return binding.key==key&&binding.modifiers==modifiers;
}
inline KeyBinding BindingFromHotkeyControl(WORD value)
{
    const BYTE flags=HIBYTE(value);
    return {LOBYTE(value),static_cast<WORD>(((flags&HOTKEYF_CONTROL)?MOD_CONTROL:0)|
            ((flags&HOTKEYF_SHIFT)?MOD_SHIFT:0)|((flags&HOTKEYF_ALT)?MOD_ALT:0))};
}
inline WORD BindingToHotkeyControl(KeyBinding binding)
{
    const BYTE flags=static_cast<BYTE>(((binding.modifiers&MOD_CONTROL)?HOTKEYF_CONTROL:0)|
         ((binding.modifiers&MOD_SHIFT)?HOTKEYF_SHIFT:0)|((binding.modifiers&MOD_ALT)?HOTKEYF_ALT:0));
    return MAKEWORD(binding.key,flags);
}
inline bool ValidatePreferences(const Preferences& p,std::wstring& error)
{
    error.clear();
    if(p.themeId<0||p.themeId>CustomThemeId){error=L"theme";return false;}
    if(p.transcriptionLanguage<0||p.transcriptionLanguage>1){error=L"transcription-language";return false;}
    if(p.opacityPercent<40||p.opacityPercent>100){error=L"opacity";return false;}
    if(p.textWeight!=400&&p.textWeight!=600&&p.textWeight!=700){error=L"weight";return false;}
    if(p.iconSize<12||p.iconSize>24){error=L"icon-size";return false;}
    for(unsigned i=0;i<PreferenceCommandCount;++i) {
        const auto b=p.shortcuts[i];
        const bool function=b.key>=VK_F1&&b.key<=VK_F24;
        const bool letter=(b.key>=L'A'&&b.key<=L'Z')||(b.key>=L'0'&&b.key<=L'9');
        const bool symbol=b.key>=VK_OEM_1&&b.key<=VK_OEM_3;
        const bool keypad=b.key>=VK_NUMPAD0&&b.key<=VK_DIVIDE;
        const bool supported=function||letter||symbol||keypad||b.key==VK_OEM_4||b.key==VK_OEM_5||
            b.key==VK_OEM_6||b.key==VK_OEM_7||b.key==VK_RETURN||b.key==VK_SPACE||
            b.key==VK_HOME||b.key==VK_END||b.key==VK_PRIOR||b.key==VK_NEXT||b.key==VK_INSERT;
        const bool contextSpace=i==static_cast<unsigned>(PreferenceCommand::AutoScroll)&&b.key==VK_SPACE&&!b.modifiers;
        if(!supported||(b.modifiers&~(MOD_CONTROL|MOD_SHIFT|MOD_ALT))||(!b.modifiers&&!function&&!contextSpace)) {
            error=L"key";return false;
        }
        // Preserve system switching/emergency controls and common editor/browser commands.
        const bool system=b.key==VK_F12||(b.key==VK_F4&&(b.modifiers&MOD_ALT))||
                          (b.key==VK_SPACE&&(b.modifiers&MOD_ALT));
        const bool standard=(b.modifiers==MOD_CONTROL||b.modifiers==(MOD_CONTROL|MOD_SHIFT))&&
            (b.key==L'A'||b.key==L'C'||b.key==L'V'||b.key==L'X'||b.key==L'Z'||b.key==L'Y'||
             b.key==L'F'||b.key==L'W'||b.key==L'T'||b.key==L'N'||b.key==L'L'||b.key==L'R'||
             b.key==L'0'||b.key==VK_OEM_PLUS||b.key==VK_OEM_MINUS||b.key==VK_ADD||b.key==VK_SUBTRACT);
        if(system||standard){error=L"reserved";return false;}
        // A global binding must not claim ordinary typing, navigation or the space bar.
        if(i<2&&!function&&!(b.modifiers&(MOD_CONTROL|MOD_ALT))){error=L"global";return false;}
        for(unsigned j=0;j<i;++j)if(b==p.shortcuts[j]){error=L"duplicate";return false;}
    }
    return true;
}
inline std::wstring ShortcutLabel(KeyBinding binding)
{
    std::wstring label;
    if(binding.modifiers&MOD_CONTROL)label+=L"Ctrl+";
    if(binding.modifiers&MOD_ALT)label+=L"Alt+";
    if(binding.modifiers&MOD_SHIFT)label+=L"Shift+";
    UINT scan=MapVirtualKeyW(binding.key,MAPVK_VK_TO_VSC);
    if(binding.key==VK_INSERT||binding.key==VK_HOME||binding.key==VK_END||
       binding.key==VK_PRIOR||binding.key==VK_NEXT)scan|=0x100;
    wchar_t name[64]{};
    if(GetKeyNameTextW(static_cast<LONG>(scan<<16),name,64))label+=name;
    else if(binding.key>=L'A'&&binding.key<=L'Z')label+=static_cast<wchar_t>(binding.key);
    else label+=L"#"+std::to_wstring(binding.key);
    return label;
}
// Fixed record: magic, version, count, opacity, weight, then ten (key, modifiers) pairs.
// V1: 100 bytes. V2: icon/browser options, 116 bytes. V3: workspace switches, 124 bytes.
// V4: theme, 128 bytes. V5: transcription language and tab labels, 140 bytes.
// This file contains no document paths, browser data, account IDs or content.
inline bool SavePreferences(const std::wstring& dataDirectory,const Preferences& p)
{
    std::wstring error;
    if(dataDirectory.empty()||!ValidatePreferences(p,error))return false;
    std::array<DWORD,15+2*PreferenceCommandCount> values{{0x50515351,5,PreferenceCommandCount,
        static_cast<DWORD>(p.opacityPercent),static_cast<DWORD>(p.textWeight)}};
    for(unsigned i=0;i<PreferenceCommandCount;++i){values[5+i*2]=p.shortcuts[i].key;values[6+i*2]=p.shortcuts[i].modifiers;}
    constexpr unsigned appearance=5+2*PreferenceCommandCount;
    values[appearance]=static_cast<DWORD>(p.iconSize);values[appearance+1]=p.browserMuted;
    values[appearance+2]=p.browserAllowCamera;values[appearance+3]=p.browserAllowMicrophone;
    values[appearance+4]=p.rememberWindowSize;values[appearance+5]=p.allowFileEditing;
    values[appearance+6]=p.themeId;values[appearance+7]=static_cast<DWORD>(p.transcriptionLanguage);
    values[appearance+8]=p.numberedDocumentTabs;values[appearance+9]=p.numberedBrowserTabs;
    const auto target=dataDirectory+L"\\preferences.dat",temporary=target+L".tmp";
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;
    bool ok=WriteFile(file,values.data(),sizeof(values),&written,nullptr)&&written==sizeof(values)&&FlushFileBuffers(file);
    CloseHandle(file);
    if(ok)ok=MoveFileExW(temporary.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
    if(!ok)DeleteFileW(temporary.c_str());
    return ok;
}
inline bool LoadPreferences(const std::wstring& dataDirectory,Preferences& p)
{
    if(dataDirectory.empty())return false;
    HANDLE file=CreateFileW((dataDirectory+L"\\preferences.dat").c_str(),GENERIC_READ,FILE_SHARE_READ,
        nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    std::array<DWORD,15+2*PreferenceCommandCount> values{};
    LARGE_INTEGER size{};DWORD read=0;
    constexpr DWORD legacyBytes=(5+2*PreferenceCommandCount)*sizeof(DWORD);
    constexpr DWORD version2Bytes=(9+2*PreferenceCommandCount)*sizeof(DWORD);
    constexpr DWORD version3Bytes=124;
    constexpr DWORD version4Bytes=128;
    const bool ok=GetFileSizeEx(file,&size)&&(size.QuadPart==legacyBytes||size.QuadPart==version2Bytes||size.QuadPart==version3Bytes||size.QuadPart==version4Bytes||size.QuadPart==sizeof(values))&&
        ReadFile(file,values.data(),static_cast<DWORD>(size.QuadPart),&read,nullptr)&&read==size.QuadPart;
    CloseHandle(file);
    const bool legacy=values[1]==1&&size.QuadPart==legacyBytes;
    const bool version2=values[1]==2&&size.QuadPart==version2Bytes;
    const bool version3=values[1]==3&&size.QuadPart==version3Bytes;
    const bool version4=values[1]==4&&size.QuadPart==version4Bytes;
    const bool current=values[1]==5&&size.QuadPart==sizeof(values);
    if(!ok||values[0]!=0x50515351||(!legacy&&!version2&&!version3&&!version4&&!current)||values[2]!=PreferenceCommandCount||
       values[3]>100||values[4]>700)return false;
    Preferences next;next.opacityPercent=static_cast<int>(values[3]);next.textWeight=static_cast<int>(values[4]);
    if(current||version4||version3||version2) {
        constexpr unsigned appearance=5+2*PreferenceCommandCount;
        if(values[appearance]>24||values[appearance+1]>1||values[appearance+2]>1||values[appearance+3]>1)return false;
        next.iconSize=static_cast<int>(values[appearance]);next.browserMuted=values[appearance+1]!=0;
        next.browserAllowCamera=values[appearance+2]!=0;next.browserAllowMicrophone=values[appearance+3]!=0;
        if(current||version4||version3){
            if(values[appearance+4]>1||values[appearance+5]>1)return false;
            next.rememberWindowSize=values[appearance+4]!=0;next.allowFileEditing=values[appearance+5]!=0;
        }
    }
    if(current||version4){if(values[31]>CustomThemeId)return false;next.themeId=static_cast<int>(values[31]);}
    if(current){
        if(values[32]>1||values[33]>1||values[34]>1)return false;
        next.transcriptionLanguage=static_cast<int>(values[32]);
        next.numberedDocumentTabs=values[33]!=0;
        next.numberedBrowserTabs=values[34]!=0;
    }
    for(unsigned i=0;i<PreferenceCommandCount;++i) {
        if(values[5+i*2]>255||values[6+i*2]>255)return false;
        next.shortcuts[i]={static_cast<WORD>(values[5+i*2]),static_cast<WORD>(values[6+i*2])};
    }
    std::wstring error;if(!ValidatePreferences(next,error))return false;
    p=next;return true;
}

// Reserve new OS hotkeys before releasing old ones. A rejected edit cannot remove
// an already available emergency mask, even if the second registration conflicts.
class PreferenceGlobalHotkeys {
public:
    using RegisterFunction=BOOL (WINAPI*)(HWND,int,UINT,UINT);
    using UnregisterFunction=BOOL (WINAPI*)(HWND,int);
    explicit PreferenceGlobalHotkeys(RegisterFunction reg=RegisterHotKey,UnregisterFunction unreg=UnregisterHotKey)
        :registerKey(reg),unregisterKey(unreg){}
    PreferenceGlobalHotkeys(const PreferenceGlobalHotkeys&)=delete;
    PreferenceGlobalHotkeys& operator=(const PreferenceGlobalHotkeys&)=delete;
    ~PreferenceGlobalHotkeys(){Release();}
    bool Initialize(HWND owner)
    {
        Release();window=owner;requested=DefaultPreferences();
        for(unsigned i=0;i<2;++i) {
            const auto binding=requested.shortcuts[i];
            if(window&&registerKey(window,Id(i),binding.modifiers|MOD_NOREPEAT,binding.key))slots[i]={binding,true};
        }
        return Registered(PreferenceCommand::Mask)&&Registered(PreferenceCommand::Recall);
    }
    bool Initialize(HWND owner,Preferences& preferences,std::wstring& error)
    {
        if(!ValidatePreferences(preferences,error)) {
            preferences=DefaultPreferences();Initialize(owner);return false;
        }
        Initialize(owner);
        if(Apply(preferences,error)) {
            if(!Registered(PreferenceCommand::Mask)){error=L"mask-conflict";return false;}
            if(!Registered(PreferenceCommand::Recall)){error=L"recall-conflict";return false;}
            return true;
        }
        // Invalid/unavailable saved global edits retain the default safety keys;
        // appearance and compatible local command preferences remain intact.
        preferences.shortcuts[0]=requested.shortcuts[0];preferences.shortcuts[1]=requested.shortcuts[1];
        // A local command may have claimed a default global chord while that
        // global used a custom binding. Restore a valid complete binding set if
        // falling back to the safety keys now creates such a collision.
        std::wstring fallbackError;
        if(!ValidatePreferences(preferences,fallbackError))preferences.shortcuts=DefaultPreferences().shortcuts;
        requested=preferences;return false;
    }
    bool Apply(const Preferences& next,std::wstring& error)
    {
        return Apply(next,error,[](){return true;});
    }
    template<typename Commit>
    bool Apply(const Preferences& next,std::wstring& error,Commit commit)
    {
        if(!ValidatePreferences(next,error))return false;
        if(!window){error=L"window";return false;}
        std::array<bool,4> staged{};
        for(unsigned command=0;command<2;++command) {
            const auto binding=next.shortcuts[command];
            bool found=false;for(const auto& slot:slots)if(slot.active&&slot.binding==binding)found=true;
            if(found)continue;
            // An unchanged startup conflict keeps its explicit local fallback;
            // it must not prevent unrelated appearance or local shortcut edits.
            if(binding==requested.shortcuts[command])continue;
            unsigned free=0;while(free<slots.size()&&slots[free].active)++free;
            if(free==slots.size()||!registerKey(window,Id(free),binding.modifiers|MOD_NOREPEAT,binding.key)) {
                for(unsigned i=0;i<slots.size();++i)if(staged[i]){unregisterKey(window,Id(i));slots[i]={};}
                error=command?L"recall-conflict":L"mask-conflict";return false;
            }
            slots[free]={binding,true};staged[free]=true;
        }
        // Keep previous safety bindings reserved until durable storage succeeds.
        // No rollback registration race is possible when a disk write fails.
        if(!commit()) {
            for(unsigned i=0;i<slots.size();++i)if(staged[i]){unregisterKey(window,Id(i));slots[i]={};}
            error=L"storage";return false;
        }
        for(unsigned i=0;i<slots.size();++i)if(slots[i].active&&slots[i].binding!=next.shortcuts[0]&&slots[i].binding!=next.shortcuts[1]) {
            unregisterKey(window,Id(i));slots[i]={};
        }
        requested=next;error.clear();return true;
    }
    bool Registered(PreferenceCommand command)const
    {
        if(static_cast<unsigned>(command)>1)return false;
        for(const auto& slot:slots)if(slot.active&&slot.binding==requested.Key(command))return true;
        return false;
    }
    const Preferences& Current()const{return requested;}
    bool CommandForId(int id,PreferenceCommand& command,LPARAM keyData=0)const
    {
        for(unsigned i=0;i<slots.size();++i)if(Id(i)==id&&slots[i].active) {
            if(keyData&&(HIWORD(keyData)!=slots[i].binding.key||LOWORD(keyData)!=slots[i].binding.modifiers))return false;
            for(unsigned c=0;c<2;++c)if(slots[i].binding==requested.shortcuts[c]){command=static_cast<PreferenceCommand>(c);return true;}
        }
        return false;
    }
    void Release()
    {
        for(unsigned i=0;i<slots.size();++i)if(slots[i].active){unregisterKey(window,Id(i));slots[i]={};}
        window=nullptr;
    }
private:
    struct Slot {KeyBinding binding{};bool active=false;};
    static int Id(unsigned slot){return 2100+static_cast<int>(slot);}
    HWND window=nullptr;
    std::array<Slot,4> slots{};
    Preferences requested=DefaultPreferences();
    RegisterFunction registerKey;UnregisterFunction unregisterKey;
};
}
