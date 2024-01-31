#include "libfiledetailer.h"
#include "FileDetailer.h"

#include <string.h>

#include <unordered_map>
#include <memory>

std::unordered_map<FDHandle, std::unique_ptr<CFileDetailer>> kFdStore;
std::string strGlobalLastErrorString;

FDHandle FileDetailer_Create()
{
    strGlobalLastErrorString = "";

    auto spFileDetailer = std::make_unique<CFileDetailer>();

    if (!spFileDetailer->GetIsComInitializied())
    {
        strGlobalLastErrorString = spFileDetailer->ErrorCodeToString(spFileDetailer->GetLastError());
        return 0;
    }

    auto ulFileDetailerHandle = reinterpret_cast<FDHandle>(spFileDetailer.get());
    kFdStore.insert(std::pair<FDHandle, std::unique_ptr<CFileDetailer>>(ulFileDetailerHandle, std::move(spFileDetailer)));

    return ulFileDetailerHandle;
}

void FileDetailer_Free(const FDHandle hFd)
{
    kFdStore.erase(hFd);
}

EFDErrorCode FileDetailer_GetFileDetailW(const FDHandle hFd, const wchar_t* wszFilePath)
{
    const auto& spFileDetailer = kFdStore[hFd];

    if (spFileDetailer == nullptr)
    {
        return FD_NotCreated;
    }

    if (!spFileDetailer->GetFileDetail(wszFilePath))
    {
        return FD_CallError;
    }

    return FD_CallSuccess;
}


EFDErrorCode FileDetailer_GetFileDetailA(const FDHandle hFd, const char* szFilePath)
{
    const auto& spFileDetailer = kFdStore[hFd];

    if (spFileDetailer == nullptr)
    {
        return FD_NotCreated;
    }

    if (!spFileDetailer->GetFileDetail(szFilePath))
    {
        return FD_CallError;
    }

    return FD_CallSuccess;
}

EFDErrorCode FileDetailer_GetLastErrorString(const FDHandle hFd, char** pszErrorString)
{
    if (hFd == static_cast<FDHandle>(0))
    {
        if (strGlobalLastErrorString.length())
        {
            *pszErrorString = new char[strGlobalLastErrorString.length() + 1];
            memset(*pszErrorString, 0, strGlobalLastErrorString.length() + 1);
            memcpy(*pszErrorString, strGlobalLastErrorString.data(), strGlobalLastErrorString.length());
        }
        return FD_CallSuccess;
    }

    const auto& spFileDetailer = kFdStore[hFd];

    if (spFileDetailer == nullptr)
    {
        return FD_NotCreated;
    }

    const auto strLastError = spFileDetailer->ErrorCodeToString(spFileDetailer->GetLastError());
    *pszErrorString = new char[strLastError.length() + 1];
    memset(*pszErrorString, 0, strLastError.length() + 1);
    memcpy(*pszErrorString, strLastError.data(), strLastError.length());

    return FD_CallSuccess;
}

EFDErrorCode FileDetailer_GetLastDetails(const FDHandle hFd, CFDDetails** ppkFdDetails)
{
    const auto& spFileDetailer = kFdStore[hFd];

    if (spFileDetailer == nullptr)
    {
        return FD_NotCreated;
    }

    auto kDetails = spFileDetailer->GetLastDetails();

    CFDDetails* pkFdDetailCurrentNode = nullptr;
    CFDDetails* pkFdDetailTopNode = nullptr;

    for (const auto& kDetail : kDetails)
    {
        CFDDetails* pkDetailNewNode = new CFDDetails{ nullptr, nullptr, nullptr };

        pkDetailNewNode->szPropKey = new char[kDetail.first.length() + 1];
        memset(pkDetailNewNode->szPropKey, 0, kDetail.first.length() + 1);
        memcpy(pkDetailNewNode->szPropKey, kDetail.first.data(), kDetail.first.length());

        pkDetailNewNode->szPropValue = new char[kDetail.second.length() + 1];
        memset(pkDetailNewNode->szPropValue, 0, kDetail.second.length() + 1);
        memcpy(pkDetailNewNode->szPropValue, kDetail.second.data(), kDetail.second.length());

        pkDetailNewNode->next = nullptr;

        if (pkFdDetailCurrentNode == nullptr)
        {
            pkFdDetailCurrentNode = pkDetailNewNode;
            pkFdDetailTopNode = pkFdDetailCurrentNode;
        }
        else
        {
            pkFdDetailCurrentNode->next = pkDetailNewNode;
            pkFdDetailCurrentNode = pkFdDetailCurrentNode->next;
        }
    }

    *ppkFdDetails = pkFdDetailTopNode;

    return FD_CallSuccess;
}

void FileDetailer_FreeDetailsBuffer(CFDDetails** ppkFdDetails)
{
    if (ppkFdDetails == nullptr || *ppkFdDetails == nullptr)
    {
        return;
    }

    CFDDetails* pkFdDetailCurrentNode = *ppkFdDetails;

    while (pkFdDetailCurrentNode != nullptr)
    {
        CFDDetails* pkFdDetailNextNode = pkFdDetailCurrentNode->next;

        if (pkFdDetailCurrentNode->szPropKey != nullptr)
        {
            delete[] pkFdDetailCurrentNode->szPropKey;
            pkFdDetailCurrentNode->szPropKey = nullptr;
        }

        if (pkFdDetailCurrentNode->szPropValue != nullptr)
        {
            delete[] pkFdDetailCurrentNode->szPropValue;
            pkFdDetailCurrentNode->szPropValue = nullptr;
        }

        delete pkFdDetailCurrentNode;

        pkFdDetailCurrentNode = pkFdDetailNextNode;
    }

    *ppkFdDetails = nullptr;
}

void FileDetailer_FreeStringBuffer(char** pBuffer)
{
    if (pBuffer == nullptr || *pBuffer == nullptr)
    {
        return;
    }

    delete[] * pBuffer;
    *pBuffer = nullptr;
}