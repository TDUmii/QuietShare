#pragma once
namespace quietshare {
inline constexpr wchar_t SpeechArchiveUrl[]=L"https://github.com/TDUmii/QuietShare/releases/download/v0.20.0/QuietShare-Speech-CPU-x64.zip";
inline constexpr wchar_t SpeechArchiveHash[]=L"d916d6a352f2598dd3ae0fe1de205c8840dc254e3d6c18c803d5f1ac6135a6cc";
inline constexpr unsigned long long SpeechArchiveBytes=1488124;
inline constexpr wchar_t SpeechExecutableHash[]=L"96a4d1b1342e33d39ebf6cd38997330cc7ed869e81e4f1e215858f407132dd93";
inline constexpr wchar_t SpeechModelRevision[]=L"5359861c739e955e79d9a303bcbc70fb988958b1";
struct SpeechModelAsset {const wchar_t* name;const wchar_t* hash;unsigned long long bytes;};
inline constexpr SpeechModelAsset SpeechModels[]={
    {L"ggml-base.bin",L"60ed5bc3dd14eea856493d334349b405782ddcaf0028d4b5df4088345fba2efe",147951465},
    {L"ggml-small.bin",L"1be3a9b2063867b937e64e2ec7483364a79917e157fa98c5d94b5c1fffea987b",487601967}
};
}
