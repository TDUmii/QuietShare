#include "../src/Preferences.h"
#include <cstdio>
#include <map>
#include <vector>

using namespace quietshare;
namespace {
std::map<int,KeyBinding> registered;
std::vector<KeyBinding> unavailable;
unsigned registerCalls=0,unregisterCalls=0;
BOOL WINAPI RegisterFixture(HWND,int id,UINT modifiers,UINT key)
{
    ++registerCalls;const KeyBinding binding{static_cast<WORD>(key),static_cast<WORD>(modifiers&~MOD_NOREPEAT)};
    for(const auto b:unavailable)if(b==binding)return FALSE;
    for(const auto& entry:registered)if(entry.second==binding)return FALSE;
    registered[id]=binding;return TRUE;
}
BOOL WINAPI UnregisterFixture(HWND,int id){++unregisterCalls;return registered.erase(id)?TRUE:FALSE;}
bool Equal(const Preferences& a,const Preferences& b)
{
    return a.shortcuts==b.shortcuts&&a.opacityPercent==b.opacityPercent&&a.textWeight==b.textWeight&&
        a.iconSize==b.iconSize&&a.browserMuted==b.browserMuted&&a.browserAllowCamera==b.browserAllowCamera&&
        a.browserAllowMicrophone==b.browserAllowMicrophone;
}
bool Put(const std::wstring& path,const void* bytes,DWORD length)
{
    HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;const bool ok=WriteFile(file,bytes,length,&written,nullptr)&&written==length;
    CloseHandle(file);return ok;
}
}
int wmain()
{
    unsigned passed=0,failed=0;bool first=true;
    std::puts("{\"checks\":[");
    auto check=[&](const char* name,bool ok){
        std::printf("%s{\"name\":\"%s\",\"passed\":%s}",first?"":",\n",name,ok?"true":"false");
        first=false;ok?++passed:++failed;
    };
    std::wstring error;const auto defaults=DefaultPreferences();
    check("defaultsKeepExistingCommands",defaults.Key(PreferenceCommand::Mask)==KeyBinding{VK_F8,0}&&
        defaults.Key(PreferenceCommand::Recall)==KeyBinding{L'Q',MOD_CONTROL|MOD_ALT}&&
        defaults.Key(PreferenceCommand::Focus)==KeyBinding{VK_F11,0}&&
        defaults.Key(PreferenceCommand::OpenFile)==KeyBinding{L'O',MOD_CONTROL}&&
        defaults.Key(PreferenceCommand::OpenFolder)==KeyBinding{L'O',MOD_CONTROL|MOD_SHIFT}&&
        defaults.Key(PreferenceCommand::Explorer)==KeyBinding{L'B',MOD_CONTROL}&&
        defaults.Key(PreferenceCommand::Reading)==KeyBinding{VK_RETURN,MOD_CONTROL}&&
        defaults.Key(PreferenceCommand::AutoScroll)==KeyBinding{VK_SPACE,0});
    check("defaultsValidate",ValidatePreferences(defaults,error)&&error.empty());
    check("newPreferencesDefaultToStandardIconsAndPrivateBrowser",defaults.iconSize==16&&!defaults.browserMuted&&
        !defaults.browserAllowCamera&&!defaults.browserAllowMicrophone);
    auto edited=defaults;edited.opacityPercent=40;edited.textWeight=600;edited.iconSize=24;
    edited.browserMuted=true;edited.browserAllowCamera=true;edited.browserAllowMicrophone=true;
    edited.Key(PreferenceCommand::VerifyWgc)={L'P',MOD_CONTROL|MOD_SHIFT};
    check("customSettingsValidate",ValidatePreferences(edited,error));
    check("bindingMatchesExactModifiers",MatchShortcut(edited.Key(PreferenceCommand::VerifyWgc),L'P',MOD_CONTROL|MOD_SHIFT)&&
          !MatchShortcut(edited.Key(PreferenceCommand::VerifyWgc),L'P',MOD_CONTROL));
    check("hotkeyControlConversionPreservesModifiers",BindingFromHotkeyControl(BindingToHotkeyControl(edited.Key(PreferenceCommand::VerifyWgc)))==edited.Key(PreferenceCommand::VerifyWgc));
    check("shortcutLabelNamesModifiers",ShortcutLabel(edited.Key(PreferenceCommand::VerifyWgc)).find(L"Ctrl+Shift+")==0);
    check("shortcutLabelNamesFunctionKey",!ShortcutLabel(defaults.Key(PreferenceCommand::Mask)).empty());
    auto rejected=[&](const char* name,KeyBinding binding){auto p=defaults;p.Key(PreferenceCommand::VerifyWgc)=binding;check(name,!ValidatePreferences(p,error));};
    rejected("emptyBindingRejected",{0,0});
    rejected("ordinaryTypingRejected",{L'A',0});
    rejected("modifierOnlyRejected",{VK_CONTROL,MOD_CONTROL});
    rejected("windowsModifierRejected",{VK_F9,MOD_WIN});
    rejected("altF4Rejected",{VK_F4,MOD_ALT});
    rejected("reservedF12Rejected",{VK_F12,0});
    rejected("altTabRejected",{VK_TAB,MOD_ALT});
    rejected("ctrlTabRejected",{VK_TAB,MOD_CONTROL});
    rejected("ctrlLRejected",{L'L',MOD_CONTROL});
    rejected("ctrlFRejected",{L'F',MOD_CONTROL});
    rejected("ctrlTRejected",{L'T',MOD_CONTROL});
    rejected("ctrlWRejected",{L'W',MOD_CONTROL});
    rejected("pasteRejected",{L'V',MOD_CONTROL});
    rejected("browserResetZoomRejected",{L'0',MOD_CONTROL});
    rejected("browserZoomInRejected",{VK_OEM_PLUS,MOD_CONTROL});
    rejected("browserZoomOutRejected",{VK_SUBTRACT,MOD_CONTROL});
    rejected("duplicateBindingRejected",{VK_F8,0});
    rejected("nonContextSpaceRejected",{VK_SPACE,0});
    auto invalid=defaults;invalid.Key(PreferenceCommand::Mask)={L'K',MOD_SHIFT};
    check("globalShiftTypingRejected",!ValidatePreferences(invalid,error));
    invalid=defaults;invalid.opacityPercent=39;check("opacityLowerBound",!ValidatePreferences(invalid,error));
    invalid=defaults;invalid.opacityPercent=101;check("opacityUpperBound",!ValidatePreferences(invalid,error));
    invalid=defaults;invalid.textWeight=500;check("unknownTextWeightRejected",!ValidatePreferences(invalid,error));
    invalid=defaults;invalid.textWeight=700;check("boldWeightAccepted",ValidatePreferences(invalid,error));
    invalid=defaults;invalid.iconSize=11;check("iconSizeLowerBound",!ValidatePreferences(invalid,error)&&error==L"icon-size");
    invalid=defaults;invalid.iconSize=25;check("iconSizeUpperBound",!ValidatePreferences(invalid,error)&&error==L"icon-size");
    invalid=defaults;invalid.iconSize=12;check("minimumIconSizeAccepted",ValidatePreferences(invalid,error));
    invalid.iconSize=24;check("maximumIconSizeAccepted",ValidatePreferences(invalid,error));
    wchar_t temp[MAX_PATH]{};GetTempPathW(MAX_PATH,temp);
    const std::wstring directory=std::wstring(temp)+L"QuietShare-Preferences-"+std::to_wstring(GetCurrentProcessId());
    const std::wstring path=directory+L"\\preferences.dat";
    check("isolatedStorageCreated",CreateDirectoryW(directory.c_str(),nullptr)!=FALSE);
    Preferences loaded=defaults;
    check("missingFileLeavesCallerUnchanged",!LoadPreferences(directory,loaded)&&Equal(loaded,defaults));
    check("preferencesSavedAtomically",SavePreferences(directory,edited)&&GetFileAttributesW((path+L".tmp").c_str())==INVALID_FILE_ATTRIBUTES);
    WIN32_FILE_ATTRIBUTE_DATA info{};
    check("recordIsBoundedAndFixed",GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&info)&&info.nFileSizeLow==116&&info.nFileSizeHigh==0);
    check("settingsRoundTrip",LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    bool independentBrowserOptions=true;
    for(unsigned options=0;options<8;++options) {
        auto candidate=edited;candidate.iconSize=12;candidate.browserMuted=(options&1)!=0;
        candidate.browserAllowCamera=(options&2)!=0;candidate.browserAllowMicrophone=(options&4)!=0;
        independentBrowserOptions=SavePreferences(directory,candidate)&&LoadPreferences(directory,loaded)&&
            Equal(loaded,candidate)&&independentBrowserOptions;
    }
    check("browserOptionsRoundTripIndependentlyAtMinimumIconSize",independentBrowserOptions);
    check("customPreferencesRestored",SavePreferences(directory,edited));
    invalid=edited;invalid.opacityPercent=0;
    check("invalidSaveKeepsExistingPreferences",!SavePreferences(directory,invalid)&&LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    invalid=edited;invalid.iconSize=25;
    check("invalidIconSaveKeepsExistingPreferences",!SavePreferences(directory,invalid)&&LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    HANDLE lock=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    check("replaceFailureReportedWithoutTemporaryFile",lock!=INVALID_HANDLE_VALUE&&!SavePreferences(directory,defaults)&&
          GetFileAttributesW((path+L".tmp").c_str())==INVALID_FILE_ATTRIBUTES);
    if(lock!=INVALID_HANDLE_VALUE)CloseHandle(lock);
    check("replaceFailurePreservesPreviousFile",LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    std::array<DWORD,29> record{{0x50515351,2,PreferenceCommandCount,100,400}};
    for(unsigned i=0;i<PreferenceCommandCount;++i){record[5+i*2]=defaults.shortcuts[i].key;record[6+i*2]=defaults.shortcuts[i].modifiers;}
    record[25]=16;
    const auto validRecord=record;
    auto corrupt=[&](const char* name,unsigned index,DWORD value){record=validRecord;record[index]=value;loaded=edited;
        check(name,Put(path,record.data(),sizeof(record))&&!LoadPreferences(directory,loaded)&&Equal(loaded,edited));};
    corrupt("unknownMagicRejected",0,0);
    corrupt("unknownVersionRejected",1,99);
    corrupt("unknownCommandCountRejected",2,999);
    corrupt("storedInvalidOpacityRejected",3,0);
    corrupt("storedInvalidWeightRejected",4,500);
    corrupt("storedKeyOverflowRejectedBeforeNarrowing",5,0x10077);
    corrupt("storedModifierOverflowRejectedBeforeNarrowing",6,0x10000);
    corrupt("storedIconSizeBelowMinimumRejected",25,11);
    corrupt("storedIconSizeAboveMaximumRejected",25,25);
    corrupt("storedIconSizeOverflowRejectedBeforeNarrowing",25,0xFFFFFFFF);
    corrupt("storedInvalidBrowserMutedRejected",26,2);
    corrupt("storedInvalidCameraPermissionRejected",27,2);
    corrupt("storedInvalidMicrophonePermissionRejected",28,2);
    corrupt("storedBooleanOverflowRejected",28,0xFFFFFFFF);
    corrupt("legacyVersionWithCurrentRecordSizeRejected",1,1);
    record=validRecord;check("truncatedRecordRejected",Put(path,record.data(),sizeof(record)-1)&&!LoadPreferences(directory,loaded));
    std::array<DWORD,30> oversized{};check("oversizedRecordRejected",Put(path,oversized.data(),sizeof(oversized))&&!LoadPreferences(directory,loaded));
    std::array<DWORD,25> legacyRecord{{0x50515351,1,PreferenceCommandCount,
        static_cast<DWORD>(edited.opacityPercent),static_cast<DWORD>(edited.textWeight)}};
    for(unsigned i=0;i<PreferenceCommandCount;++i){legacyRecord[5+i*2]=edited.shortcuts[i].key;legacyRecord[6+i*2]=edited.shortcuts[i].modifiers;}
    auto migrated=edited;migrated.iconSize=16;migrated.browserMuted=false;migrated.browserAllowCamera=false;migrated.browserAllowMicrophone=false;
    loaded=edited;
    check("legacySettingsPreserveAppearanceAndShortcutsWithNewDefaults",Put(path,legacyRecord.data(),sizeof(legacyRecord))&&
        LoadPreferences(directory,loaded)&&Equal(loaded,migrated));
    check("legacySettingsUpgradeToCurrentRecordOnSave",SavePreferences(directory,loaded)&&
        GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&info)&&info.nFileSizeLow==116&&info.nFileSizeHigh==0&&
        LoadPreferences(directory,loaded)&&Equal(loaded,migrated));
    legacyRecord[1]=2;loaded=edited;
    check("currentVersionWithLegacyRecordSizeRejected",Put(path,legacyRecord.data(),sizeof(legacyRecord))&&
        !LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    legacyRecord[1]=1;legacyRecord[5]=0x10077;
    check("legacyMalformedKeyStillRejected",Put(path,legacyRecord.data(),sizeof(legacyRecord))&&
        !LoadPreferences(directory,loaded)&&Equal(loaded,edited));
    check("missingStorageDirectoryNotCreatedImplicitly",!SavePreferences(directory+L"\\missing",defaults));
    DeleteFileW(path.c_str());RemoveDirectoryW(directory.c_str());

    {
        PreferenceGlobalHotkeys globals(RegisterFixture,UnregisterFixture);
        check("globalDefaultsRegistered",globals.Initialize(reinterpret_cast<HWND>(1))&&registered.size()==2);
        PreferenceCommand command=PreferenceCommand::Count;
        check("maskIdRoutesToMask",globals.CommandForId(2100,command)&&command==PreferenceCommand::Mask);
        check("hotkeyPayloadMatchesRegisteredChord",globals.CommandForId(2100,command,MAKELPARAM(0,VK_F8))&&command==PreferenceCommand::Mask);
        check("staleHotkeyPayloadRejected",!globals.CommandForId(2100,command,MAKELPARAM(MOD_ALT,VK_F7)));
        check("recallIdRoutesToRecall",globals.CommandForId(2101,command)&&command==PreferenceCommand::Recall);
        check("unknownGlobalIdIgnored",!globals.CommandForId(9999,command));
        auto candidate=defaults;candidate.Key(PreferenceCommand::Mask)={VK_F7,0};candidate.Key(PreferenceCommand::Recall)={L'J',MOD_CONTROL|MOD_ALT};
        unavailable.push_back(candidate.Key(PreferenceCommand::Recall));
        check("globalConflictRejected",!globals.Apply(candidate,error)&&error==L"recall-conflict");
        check("secondConflictRollsBackFirstReservation",registered.size()==2&&registered.at(2100)==defaults.Key(PreferenceCommand::Mask)&&
              registered.at(2101)==defaults.Key(PreferenceCommand::Recall)&&globals.Current().shortcuts==defaults.shortcuts);
        check("maskRemainsAvailableAfterRejectedOverride",globals.Registered(PreferenceCommand::Mask)&&globals.CommandForId(2100,command)&&command==PreferenceCommand::Mask);
        unavailable.clear();
        bool commitCalled=false,oldKeysHeldDuringCommit=false;
        check("storageFailureRejectsEditBeforeReleasingOldKeys",!globals.Apply(candidate,error,[&](){
            commitCalled=true;oldKeysHeldDuringCommit=registered.count(2100)&&registered.count(2101)&&registered.size()==4;return false;
        })&&error==L"storage"&&commitCalled&&oldKeysHeldDuringCommit);
        check("storageFailureKeepsPreviousGlobalBindingsAndMapping",registered.size()==2&&registered.at(2100)==defaults.Key(PreferenceCommand::Mask)&&
              registered.at(2101)==defaults.Key(PreferenceCommand::Recall)&&globals.Current().shortcuts==defaults.shortcuts);
        unavailable.push_back(candidate.Key(PreferenceCommand::Recall));commitCalled=false;
        check("registrationConflictDoesNotCommitStorage",!globals.Apply(candidate,error,[&](){commitCalled=true;return true;})&&!commitCalled);
        unavailable.clear();
        const unsigned registrationsBefore=registerCalls;
        auto swapped=defaults;std::swap(swapped.Key(PreferenceCommand::Mask),swapped.Key(PreferenceCommand::Recall));
        check("swappingExistingGlobalBindingsDoesNotUnregisterSafety",globals.Apply(swapped,error)&&registerCalls==registrationsBefore&&registered.size()==2);
        check("swappedCommandIdRoutesNewMeaning",globals.CommandForId(2101,command)&&command==PreferenceCommand::Mask);
        check("customGlobalBindingsApplied",globals.Apply(candidate,error,[&](){commitCalled=true;return true;})&&registered.size()==2&&globals.Current().shortcuts==candidate.shortcuts);
        check("oldGlobalKeysReleasedAfterSuccess",registered.count(2100)==0&&registered.count(2101)==0);
        check("newGlobalIdsRouteBothCommands",globals.CommandForId(2102,command)&&command==PreferenceCommand::Mask&&
              globals.CommandForId(2103,command)&&command==PreferenceCommand::Recall);
        invalid=candidate;invalid.Key(PreferenceCommand::Mask)={0,0};
        check("invalidGlobalEditPreservesRegistrations",!globals.Apply(invalid,error)&&registered.size()==2&&globals.Current().shortcuts==candidate.shortcuts);
    }
    check("globalRegistrationsReleasedOnDestruction",registered.empty());
    unavailable.push_back(defaults.Key(PreferenceCommand::Recall));
    {
        PreferenceGlobalHotkeys globals(RegisterFixture,UnregisterFixture);
        check("startupRecallConflictDoesNotRemoveEmergencyMask",!globals.Initialize(reinterpret_cast<HWND>(1))&&globals.Registered(PreferenceCommand::Mask)&&
              !globals.Registered(PreferenceCommand::Recall)&&registered.size()==1);
        auto unrelated=defaults;unrelated.opacityPercent=67;unrelated.Key(PreferenceCommand::VerifyWgc)={L'K',MOD_CONTROL|MOD_SHIFT};
        bool stored=false;
        check("startupUnavailableUnchangedKeyDoesNotBlockUnrelatedEdits",globals.Apply(unrelated,error,[&](){stored=true;return true;})&&stored&&
              Equal(globals.Current(),unrelated)&&globals.Registered(PreferenceCommand::Mask)&&!globals.Registered(PreferenceCommand::Recall));
        auto unavailableEdit=unrelated;unavailableEdit.Key(PreferenceCommand::Recall)={L'J',MOD_CONTROL|MOD_ALT};
        unavailable.push_back(unavailableEdit.Key(PreferenceCommand::Recall));stored=false;
        check("changedUnavailableKeyStillRejectsWithoutStorage",!globals.Apply(unavailableEdit,error,[&](){stored=true;return true;})&&!stored&&
              Equal(globals.Current(),unrelated)&&globals.Registered(PreferenceCommand::Mask));
        unavailable.pop_back();
        globals.Release();check("explicitReleaseUnregistersRemainingKey",registered.empty());
        check("applyWithoutOwnerRejected",!globals.Apply(defaults,error)&&error==L"window");
    }
    unavailable.clear();
    unavailable.push_back(defaults.Key(PreferenceCommand::Recall));
    {
        PreferenceGlobalHotkeys globals(RegisterFixture,UnregisterFixture);
        auto saved=edited;
        check("startupReportsUnchangedUnavailableGlobalWithoutDiscardingLocalEdits",!globals.Initialize(reinterpret_cast<HWND>(1),saved,error)&&
              error==L"recall-conflict"&&Equal(saved,edited)&&Equal(globals.Current(),edited)&&globals.Registered(PreferenceCommand::Mask));
    }
    unavailable.clear();
    {
        PreferenceGlobalHotkeys globals(RegisterFixture,UnregisterFixture);
        auto saved=edited;saved.Key(PreferenceCommand::Recall)={L'J',MOD_CONTROL|MOD_ALT};
        unavailable.push_back(saved.Key(PreferenceCommand::Recall));
        check("savedConflictRestoresActualDefaultGlobalBindings",!globals.Initialize(reinterpret_cast<HWND>(1),saved,error)&&
              saved.Key(PreferenceCommand::Mask)==defaults.Key(PreferenceCommand::Mask)&&saved.Key(PreferenceCommand::Recall)==defaults.Key(PreferenceCommand::Recall));
        check("savedConflictPreservesLocalAndAppearancePreferences",saved.opacityPercent==edited.opacityPercent&&saved.textWeight==edited.textWeight&&
              saved.Key(PreferenceCommand::VerifyWgc)==edited.Key(PreferenceCommand::VerifyWgc)&&Equal(saved,globals.Current()));
        unavailable.clear();
        saved=edited;saved.opacityPercent=0;
        check("invalidStartupSettingsRestoreSafeDefaults",!globals.Initialize(reinterpret_cast<HWND>(1),saved,error)&&Equal(saved,defaults)&&
              globals.Registered(PreferenceCommand::Mask)&&globals.Registered(PreferenceCommand::Recall));
    }
    check("startupFallbackRegistrationsReleased",registered.empty());
    {
        PreferenceGlobalHotkeys globals(RegisterFixture,UnregisterFixture);
        auto saved=edited;saved.Key(PreferenceCommand::Mask)={VK_F6,0};saved.Key(PreferenceCommand::VerifyWgc)={VK_F8,0};
        check("savedCustomMaskAndLocalF8InitiallyDistinct",ValidatePreferences(saved,error));
        unavailable.push_back(saved.Key(PreferenceCommand::Mask));
        check("conflictingCustomMaskFallsBackWithoutDuplicateLocalF8",!globals.Initialize(reinterpret_cast<HWND>(1),saved,error)&&
              error==L"mask-conflict"&&saved.shortcuts==defaults.shortcuts&&ValidatePreferences(saved,error)&&
              globals.Registered(PreferenceCommand::Mask)&&saved.Key(PreferenceCommand::Mask)==KeyBinding{VK_F8,0});
        check("collisionFallbackPreservesAppearanceAndActualCurrentState",saved.opacityPercent==edited.opacityPercent&&
              saved.textWeight==edited.textWeight&&Equal(saved,globals.Current()));
        saved.opacityPercent=85;
        check("collisionFallbackStillAllowsFuturePreferenceEdits",globals.Apply(saved,error)&&Equal(saved,globals.Current())&&
              globals.Registered(PreferenceCommand::Mask));
        unavailable.clear();
    }
    check("collisionFallbackRegistrationsReleased",registered.empty());
    std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);
    return failed?1:0;
}
