#include <Napi.h>
#include <Windows.h>

#include <libfiledetailer.h>

#include <string>
#include <vector>
#include <optional>

typedef int (*pfn_StringConv_GetShellId)(char *szInputShellString, char **pszOutputShellString);
typedef void (*pfn_StringConv_FreeStringBuffer)(char **ppBuffer);

static pfn_StringConv_GetShellId StringConv_GetShellId = nullptr;
static pfn_StringConv_FreeStringBuffer StringConv_FreeStringBuffer = nullptr;

using CFileDetails = struct cFileDetails
{
    std::string strPropKey;
    std::string strPropValue;
    std::optional<std::string> strPropShellId;
};

#define CHECK_ARGUMENTS_MIN_COUNT(COUNT)                                                                                          \
    if (info.Length() < COUNT)                                                                                                    \
    {                                                                                                                             \
        Napi::TypeError::New(env, "invalid count of arguments, must at least " #COUNT " arguments").ThrowAsJavaScriptException(); \
        return env.Null();                                                                                                        \
    }

#define CHECK_ARGUMENTS_TYPE(TYPE, ASTYPE, POS)                                                                         \
    if (!info[POS].Is##TYPE())                                                                                          \
    {                                                                                                                   \
        Napi::TypeError::New(env, "parameter position " #POS " must be a " #TYPE " type").ThrowAsJavaScriptException(); \
        return env.Null();                                                                                              \
    }                                                                                                                   \
    auto arg##POS = info[POS].As<Napi::##ASTYPE>();

#define CONVERT_ARG0_TO_UHANDLE \
    uint64_t uHandle = std::stoull(arg0.Utf8Value());

#define INTERPRET_ERROR(ERRCODE, FN_NAME, CLS_NAME, ERROR_DEF, NOT_FOUND_DEF)                                                     \
    if (ERRCODE == NOT_FOUND_DEF)                                                                                                 \
    {                                                                                                                             \
        Napi::Error::New(env, "call " #FN_NAME " failed, instance of " #CLS_NAME " is not created").ThrowAsJavaScriptException(); \
        return env.Null();                                                                                                        \
    }                                                                                                                             \
    else if (ERRCODE == ERROR_DEF)                                                                                                \
    {                                                                                                                             \
        std::string strErrReason("call " #FN_NAME " failed, unknown reason");                                                     \
        char *szErrReason = nullptr;                                                                                              \
        FileDetailer_GetLastErrorString(uHandle, &szErrReason);                                                                   \
        if (szErrReason != nullptr)                                                                                               \
        {                                                                                                                         \
            strErrReason = std::string(szErrReason);                                                                              \
            FileDetailer_FreeStringBuffer(&szErrReason);                                                                          \
        }                                                                                                                         \
        Napi::Error::New(env, strErrReason).ThrowAsJavaScriptException();                                                         \
        return env.Null();                                                                                                        \
    }

#define NODE_FUNC(FUNCNAME) \
    exports.Set(Napi::String::New(env, #FUNCNAME), Napi::Function::New(env, NodeFunc_##FUNCNAME));

bool LoadShellConvContext()
{
    const auto hDll = LoadLibrary("libshellstringconv.dll");

    if (hDll == nullptr)
    {
        return false;
    }

    StringConv_GetShellId = (pfn_StringConv_GetShellId)GetProcAddress(hDll, "StringConv_GetShellId");
    StringConv_FreeStringBuffer = (pfn_StringConv_FreeStringBuffer)GetProcAddress(hDll, "StringConv_FreeStringBuffer");

    return (StringConv_GetShellId != nullptr) && (StringConv_FreeStringBuffer != nullptr);
}

Napi::Value NodeFunc_FileDetailer_Create(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    const auto hFileDetailer = FileDetailer_Create();

    if (!hFileDetailer)
    {
        char *szErrReason = nullptr;
        FileDetailer_GetLastErrorString(0, &szErrReason);

        std::string strErrReason("cannot create file detailer object");
        if (szErrReason != nullptr)
        {
            strErrReason = std::string(szErrReason);
            FileDetailer_FreeStringBuffer(&szErrReason);
            szErrReason = nullptr;
        }

        Napi::Error::New(env, strErrReason).ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::String::New(env, std::to_string(hFileDetailer));
}

Napi::Value NodeFunc_FileDetailer_Free(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    CHECK_ARGUMENTS_MIN_COUNT(1);
    CHECK_ARGUMENTS_TYPE(String, String, 0);
    CONVERT_ARG0_TO_UHANDLE;

    FileDetailer_Free(uHandle);

    return env.Null();
}

Napi::Value NodeFunc_FileDetailer_GetFileDetail(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    CHECK_ARGUMENTS_MIN_COUNT(2);
    CHECK_ARGUMENTS_TYPE(String, String, 0);
    CHECK_ARGUMENTS_TYPE(String, String, 1);
    CONVERT_ARG0_TO_UHANDLE;

    const auto strFilePath = arg1.Utf8Value();

    const auto nErrCode = FileDetailer_GetFileDetailA(uHandle, strFilePath.c_str());

    INTERPRET_ERROR(nErrCode, GetFileDetail, FileDetailer, FD_CallError, FD_NotCreated);

    return env.Null();
}

Napi::Value NodeFunc_FileDetailer_GetLastDetails(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    CHECK_ARGUMENTS_MIN_COUNT(1);
    CHECK_ARGUMENTS_TYPE(String, String, 0);
    CONVERT_ARG0_TO_UHANDLE;

    CFDDetails *pkFdDetailsResult = nullptr;
    const auto nErrCode = FileDetailer_GetLastDetails(uHandle, &pkFdDetailsResult);

    INTERPRET_ERROR(nErrCode, GetLastDetails, FileDetailer, FD_CallError, FD_NotCreated);

    CFDDetails *pkFdDetailsCurrent = pkFdDetailsResult;

    std::vector<CFileDetails> kDetails;

    while (pkFdDetailsCurrent != nullptr)
    {
        const std::string strPropKey(pkFdDetailsCurrent->szPropKey);
        const std::string strPropValue(pkFdDetailsCurrent->szPropValue);

        CFileDetails kFileDetail //
            {
                std::string(pkFdDetailsCurrent->szPropKey),
                std::string(pkFdDetailsCurrent->szPropValue),
                std::nullopt
                //
            };
        char *szShellId = nullptr;
        if (StringConv_GetShellId(pkFdDetailsCurrent->szPropKey, &szShellId))
        {
            if (szShellId != nullptr)
            {
                kFileDetail.strPropShellId = std::string(szShellId);
                StringConv_FreeStringBuffer(&szShellId);
            }
        }

        kDetails.push_back(std::move(kFileDetail));

        pkFdDetailsCurrent = pkFdDetailsCurrent->next;
    }

    FileDetailer_FreeDetailsBuffer(&pkFdDetailsResult);

    auto jskDetailsClasfObject = Napi::Object::New(env);

    auto jsIdDefindValue = Napi::Object::New(env);
    auto jsIdUndefinedValue = Napi::Object::New(env);

    for (const auto &kDetail : kDetails)
    {
        if (kDetail.strPropShellId.has_value())
        {
            jsIdDefindValue.Set(Napi::String::New(env, kDetail.strPropShellId.value()), Napi::String::New(env, kDetail.strPropValue));
        }
        else
        {
            jsIdUndefinedValue.Set(Napi::String::New(env, kDetail.strPropKey), Napi::String::New(env, kDetail.strPropValue));
        }
    }

    jskDetailsClasfObject.Set(Napi::String::New(env, "idDefinedValue"), jsIdDefindValue);
    jskDetailsClasfObject.Set(Napi::String::New(env, "idUndefinedValue"), jsIdUndefinedValue);

    return jskDetailsClasfObject;
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports)
{
    if (!LoadShellConvContext())
    {
        Napi::Error::New(env, "cannot load shell string converter dynamic library").ThrowAsJavaScriptException();
        return exports;
    }

    NODE_FUNC(FileDetailer_Create)
    NODE_FUNC(FileDetailer_Free)
    NODE_FUNC(FileDetailer_GetFileDetail)
    NODE_FUNC(FileDetailer_GetLastDetails)
    return exports;
}

NODE_API_MODULE(FileDetailer, Initialize)