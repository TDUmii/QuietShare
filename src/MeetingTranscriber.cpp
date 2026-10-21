#include "MeetingTranscriber.h"

#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
#include <algorithm>
#include <vector>

namespace quietshare {
namespace {

// MinGW's SAPI import library does not export the data symbol declared by
// sapi.h. Keep the documented SAPI format identifier local so the recognizer
// can still bind a standard WAVEFORMATEX stream without relying on that data
// symbol at link time.
const GUID kSpdfidWaveFormatEx={0xC31ADBAE,0x527F,0x4FF5,{0xA2,0x30,0xF6,0x2B,0xB6,0x1F,0xF7,0x0C}};

template <class T> void ReleaseCom(T*& value)
{
    if (value) {
        value->Release();
        value = nullptr;
    }
}

std::wstring TrimPhrase(std::wstring value)
{
    const auto isSpace=[](wchar_t c){return c==L' '||c==L'\t'||c==L'\r'||c==L'\n';};
    while(!value.empty()&&isSpace(value.front()))value.erase(value.begin());
    while(!value.empty()&&isSpace(value.back()))value.pop_back();
    return value;
}

void AppendPhrase(std::wstring& transcript,const std::wstring& phrase)
{
    const auto clean=TrimPhrase(phrase);
    if(clean.empty())return;
    if(!transcript.empty())transcript+=L"\r\n";
    transcript+=clean;
}

std::wstring TranscriptPath(const std::wstring& wavPath)
{
    const auto slash=wavPath.find_last_of(L"\\/");
    const auto dot=wavPath.find_last_of(L'.');
    if(dot==std::wstring::npos||(slash!=std::wstring::npos&&dot<slash))return wavPath+L".txt";
    return wavPath.substr(0,dot)+L".txt";
}

bool WriteUtf8(const std::wstring& path,const std::wstring& text)
{
    const int length=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);
    if(length<0)return false;
    std::vector<BYTE> bytes(static_cast<size_t>(length));
    if(length&&!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),
                                    reinterpret_cast<char*>(bytes.data()),length,nullptr,nullptr))return false;
    const auto temporary=path+L".tmp";
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;const bool ok=(!bytes.empty()?WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr)&&written==bytes.size():true);
    if(!CloseHandle(file)||!ok){DeleteFileW(temporary.c_str());return false;}
    if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){
        DeleteFileW(temporary.c_str());return false;
    }
    return true;
}

}

MeetingTranscriber::~MeetingTranscriber()
{
    Stop();
}

bool MeetingTranscriber::Start(const std::wstring& wavPath,TranscriptionLanguage nextLanguage)
{
    Stop();
    if(wavPath.empty()||GetFileAttributesW(wavPath.c_str())==INVALID_FILE_ATTRIBUTES){errorCode.store(1);return false;}
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        transcript.clear();outputPath.clear();language=nextLanguage;
    }
    errorCode.store(0);stopRequested.store(false);completed.store(false);consumed.store(false);running.store(true);
    worker=std::thread([this,wavPath,nextLanguage]{Run(wavPath,nextLanguage);});
    return true;
}

void MeetingTranscriber::Stop()
{
    stopRequested.store(true);
    if(worker.joinable())worker.join();
    running.store(false);
}

void MeetingTranscriber::Run(const std::wstring& wavPath,TranscriptionLanguage nextLanguage)
{
    std::wstring text;int code=0;const bool recognized=Recognize(wavPath,nextLanguage,text,code);
    std::wstring textPath;
    if(recognized&&!WriteTranscript(wavPath,text,textPath))code=7;
    if(stopRequested.load()&&code==0)code=6;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        transcript=std::move(text);outputPath=std::move(textPath);language=nextLanguage;
    }
    errorCode.store(code);completed.store(true);running.store(false);
}

bool MeetingTranscriber::Recognize(const std::wstring& wavPath,TranscriptionLanguage nextLanguage,
                                   std::wstring& result,int& code)
{
    result.clear();code=0;
    const HRESULT initialized=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    if(FAILED(initialized)){code=2;return false;}
    IEnumSpObjectTokens* tokens=nullptr;ISpObjectToken* recognizerToken=nullptr;ISpRecognizer* recognizer=nullptr;
    ISpStream* stream=nullptr;ISpRecoContext* context=nullptr;ISpRecoGrammar* grammar=nullptr;
    bool ok=false,ended=false;ULONG count=0,fetched=0;
    const wchar_t* languageAttribute=nextLanguage==TranscriptionLanguage::English?L"Language=409":L"Language=42A";
    HRESULT hr=SpEnumTokens(SPCAT_RECOGNIZERS,languageAttribute,nullptr,&tokens);
    if(FAILED(hr)||!tokens){code=4;goto finish;}
    if(FAILED(tokens->GetCount(&count))||count==0){code=4;goto finish;}
    if(FAILED(tokens->Next(1,&recognizerToken,&fetched))||fetched!=1||!recognizerToken){code=4;goto finish;}
    hr=CoCreateInstance(CLSID_SpInprocRecognizer,nullptr,CLSCTX_INPROC_SERVER,__uuidof(ISpRecognizer),reinterpret_cast<void**>(&recognizer));
    if(FAILED(hr)||!recognizer){code=3;goto finish;}
    if(FAILED(hr=recognizer->SetRecognizer(recognizerToken))){code=4;goto finish;}
    hr=CoCreateInstance(CLSID_SpStream,nullptr,CLSCTX_INPROC_SERVER,__uuidof(ISpStream),reinterpret_cast<void**>(&stream));
    if(FAILED(hr)||!stream){code=3;goto finish;}
    if(FAILED(hr=stream->BindToFile(wavPath.c_str(),SPFM_OPEN_READONLY,&kSpdfidWaveFormatEx,nullptr,SPFEI_ALL_EVENTS))){code=3;goto finish;}
    if(FAILED(hr=recognizer->SetInput(stream,FALSE))){code=3;goto finish;}
    if(FAILED(hr=recognizer->CreateRecoContext(&context))||!context){code=3;goto finish;}
    if(FAILED(hr=context->CreateGrammar(0,&grammar))||!grammar||
       FAILED(hr=grammar->LoadDictation(nullptr,SPLO_STATIC))||
       FAILED(hr=grammar->SetDictationState(SPRS_ACTIVE))){code=5;goto finish;}
    if(FAILED(hr=context->SetInterest(SPFEI(SPEI_RECOGNITION)|SPFEI(SPEI_END_INPUT_STREAM),0))||
       FAILED(hr=context->SetNotifyWin32Event())||FAILED(hr=recognizer->SetRecoState(SPRST_ACTIVE))){code=5;goto finish;}
    while(!stopRequested.load()&&!ended){
        context->WaitForNotifyEvent(100);
        SPEVENT events[16]{};ULONG fetchedEvents=0;
        if(FAILED(context->GetEvents(16,events,&fetchedEvents))){code=5;goto finish;}
        for(ULONG index=0;index<fetchedEvents;++index){
            const auto& event=events[index];
            if(event.eEventId==SPEI_RECOGNITION&&event.lParam){
                auto* phrase=reinterpret_cast<ISpRecoResult*>(event.lParam);LPWSTR text=nullptr;
                if(SUCCEEDED(phrase->GetText(0,SP_GETWHOLEPHRASE,TRUE,&text,nullptr))&&text){AppendPhrase(result,text);CoTaskMemFree(text);}
                phrase->Release();
            } else if(event.eEventId==SPEI_END_INPUT_STREAM)ended=true;
        }
    }
    if(stopRequested.load()){code=6;goto finish;}
    if(!ended){code=5;goto finish;}
    ok=true;
finish:
    if(recognizer)recognizer->SetRecoState(SPRST_INACTIVE);
    if(stream)stream->Close();
    ReleaseCom(grammar);ReleaseCom(context);ReleaseCom(stream);ReleaseCom(recognizer);ReleaseCom(recognizerToken);ReleaseCom(tokens);
    CoUninitialize();
    return ok;
}

bool MeetingTranscriber::WriteTranscript(const std::wstring& wavPath,const std::wstring& text,std::wstring& textPath)
{
    textPath=TranscriptPath(wavPath);
    return WriteUtf8(textPath,text);
}

bool MeetingTranscriber::TakeResult(std::wstring& text,std::wstring& textPath)
{
    if(!Completed()||consumed.exchange(true))return false;
    std::lock_guard<std::mutex> lock(stateMutex);text=transcript;textPath=outputPath;return true;
}

std::wstring MeetingTranscriber::OutputPath() const
{
    std::lock_guard<std::mutex> lock(stateMutex);return outputPath;
}

std::wstring MeetingTranscriber::Status(bool english) const
{
    const auto languageName=language==TranscriptionLanguage::English?(english?L"English":L"English"):
        (english?L"Vietnamese":L"Tiếng Việt");
    if(Running())return (english?L"Transcribing meeting audio":L"Đang chép lời cuộc họp")+std::wstring(L" · ")+languageName;
    switch(ErrorCode()){
    case 1:return (english?L"QS-R001 · The WAV recording could not be found.":L"QS-R001 · Không tìm thấy file WAV ghi âm.");
    case 2:return (english?L"QS-R002 · Windows Speech API could not start.":L"QS-R002 · Windows Speech API không khởi động được.");
    case 3:return (english?L"QS-R003 · The WAV format is not supported by the recognizer.":L"QS-R003 · Bộ nhận dạng không hỗ trợ định dạng WAV này.");
    case 4:return english?(language==TranscriptionLanguage::English?L"QS-R004 · English speech recognition is not installed.":L"QS-R004 · Vietnamese speech recognition is not installed."):
        (language==TranscriptionLanguage::English?L"QS-R004 · Chưa cài bộ nhận dạng giọng nói tiếng Anh.":L"QS-R004 · Chưa cài bộ nhận dạng giọng nói tiếng Việt.");
    case 5:return english?L"QS-R005 · Windows could not recognize the meeting audio.":L"QS-R005 · Windows không thể nhận dạng âm thanh cuộc họp.";
    case 6:return english?L"QS-R006 · Transcription was stopped.":L"QS-R006 · Đã dừng chép lời.";
    case 7:return english?L"QS-R007 · The transcript could not be saved next to the recording.":L"QS-R007 · Không lưu được file chép lời cạnh bản ghi.";
    default:return {};
    }
}

}
