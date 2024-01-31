#include "FileDetailer.h"
#include <type_traits>

#include <string.h>

#include <filesystem>

namespace fs = std::filesystem;



CFileDetailer::CFileDetailer()
{
    CLSID kClsid;
    HRESULT hr;

    if (SUCCEEDED((hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED))))
    {
        if (SUCCEEDED((hr = CLSIDFromProgID(L"Shell.Application", &kClsid))))
        {
            CComPtr<IShellDispatch> spDispatch = nullptr;
            if (SUCCEEDED((hr = spDispatch.CoCreateInstance(kClsid, nullptr, CLSCTX_ALL))))
            {
                m_spDispatch = std::move(spDispatch);
                m_bIsComInitialized = true;
            }
            else
            {
                m_eLastErrorCode = EFileDetailerErrorCode::ComCreateIDispatchFailed;
            }
        }
        else
        {
            m_eLastErrorCode = EFileDetailerErrorCode::ComGetClsIDFailed;
        }
    }
    else
    {
        m_eLastErrorCode = EFileDetailerErrorCode::ComInitializeFailed;
    }
}


CFileDetailer::~CFileDetailer()
{
    CoUninitialize();
}

bool CFileDetailer::GetFileDetail(const std::wstring& wstrFilePath)
{
    m_eLastErrorCode = EFileDetailerErrorCode::NoError;

    if (!m_bIsComInitialized)
    {
        m_eLastErrorCode = EFileDetailerErrorCode::ComNotInitialize;
        return false;
    }

    const fs::path kTargetPath(wstrFilePath);

    std::wstring wstrTargetFileName(kTargetPath.filename().wstring());

    HRESULT hr;

    CComVariant vtNamespace(kTargetPath.parent_path().wstring().c_str());
    CComPtr<Folder> spFolder = nullptr;
    if (SUCCEEDED((hr = m_spDispatch->NameSpace(vtNamespace, &spFolder))))
    {
        CComPtr<FolderItems> spFolderItems = nullptr;
        if (SUCCEEDED((hr = spFolder->Items(&spFolderItems))))
        {
            long lFolderItemsCount = 0;
            if (SUCCEEDED((hr = spFolderItems->get_Count(&lFolderItemsCount))))
            {
                CComPtr<FolderItem> spTargetImageItem = nullptr;
                if (lFolderItemsCount > 0)
                {
                    for (long i = 0; i < lFolderItemsCount; ++i)
                    {
                        CComVariant vtFolderNo(i);
                        CComPtr<FolderItem> spFolderItem = nullptr;
                        if (SUCCEEDED((hr = spFolderItems->Item(vtFolderNo, &spFolderItem))))
                        {
                            CComBSTR bstrItemName;
                            if (SUCCEEDED((hr = spFolderItem->get_Name(&bstrItemName))))
                            {
                                std::wstring wstrItemName(bstrItemName.m_str);
                                if (wstrItemName == wstrTargetFileName)
                                {
                                    spTargetImageItem = std::move(spFolderItem);
                                    break;
                                }
                            }
                        }
                    }

                    if (spTargetImageItem != nullptr)
                    {
                        m_kLastDetails.resize(0);
                        m_kLastDetails.shrink_to_fit();

                        for (int i = 0; i < 266; ++i)
                        {
                            CComVariant vtItem(spTargetImageItem);
                            CComBSTR bstrDetails;
                            if (SUCCEEDED((hr = spFolder->GetDetailsOf(vtItem, i, &bstrDetails))))
                            {
                                CComVariant vtItems(spFolderItems);
                                CComBSTR bstrPropKey;
                                if (SUCCEEDED((hr = spFolder->GetDetailsOf(vtItems, i, &bstrPropKey))))
                                {

                                    auto strPropKey = ConvertString<std::string, std::wstring>(bstrPropKey.m_str);
                                    auto strValue = ConvertString<std::string, std::wstring>(bstrDetails.m_str);

                                    if (strPropKey.length())
                                    {
                                        m_kLastDetails.emplace_back(std::move(strPropKey), std::move(strValue));
                                    }
                                }
                            }
                        }

                        return true;
                    }
                    else
                    {
                        m_eLastErrorCode = EFileDetailerErrorCode::ComTargetFilePathNotFoundFailed;
                    }
                }
                else
                {
                    m_eLastErrorCode = EFileDetailerErrorCode::ComEmptyFolderItemsFailed;
                }
            }
            else
            {
                m_eLastErrorCode = EFileDetailerErrorCode::ComEnumFolderItemsCountFailed;
            }
        }
        else
        {
            m_eLastErrorCode = EFileDetailerErrorCode::ComEnumFolderItemsFailed;
        }
    }
    else
    {
        m_eLastErrorCode = EFileDetailerErrorCode::ComCallNamespaceFailed;
    }

    return false;
}

bool CFileDetailer::GetFileDetail(const std::string& strFilePath)
{
    const auto wstrFilePath = ConvertString<std::wstring, std::string>(strFilePath);
    return GetFileDetail(wstrFilePath);
}

std::string CFileDetailer::ErrorCodeToString(const EFileDetailerErrorCode eCode)
{
    switch (eCode)
    {
    case EFileDetailerErrorCode::TargetFileNotExists:
    {
        return "target file path is not exists";
    }
    case EFileDetailerErrorCode::ComNotInitialize:
    {
        return "com library is not initialized";
    }
    case EFileDetailerErrorCode::ComInitializeFailed:
    {
        return "com library is not initialized";
    }
    case EFileDetailerErrorCode::ComGetClsIDFailed:
    {
        return "com clsid get failed";
    }
    case EFileDetailerErrorCode::ComCreateIDispatchFailed:
    {
        return "com idispatch interface create failed";
    }
    case EFileDetailerErrorCode::ComCallNamespaceFailed:
    {
        return "com idispatch->namespace method call failed";
    }
    case EFileDetailerErrorCode::ComEnumFolderItemsFailed:
    {
        return "com idispatch->items enumerate items call failed";
    }
    case EFileDetailerErrorCode::ComEnumFolderItemsCountFailed:
    {
        return "com folderitems get items count failed";
    }
    case EFileDetailerErrorCode::ComEmptyFolderItemsFailed:
    {
        return "com folderitems empty items failed";
    }
    case EFileDetailerErrorCode::ComTargetFilePathNotFoundFailed:
    {
        return "com cannot find target file path";
    }
    }

    return "no error";
}

template <typename Dst, typename Src>
Dst CFileDetailer::ConvertString(const Src& kInputString)
{
    Dst kOutputString;

    if constexpr (std::is_same_v<Dst, Src>)
    {
        kOutputString = kInputString;
    }
    else if constexpr (std::is_same_v<Dst, std::string>)
    {
        auto size = WideCharToMultiByte(CP_UTF8, 0, kInputString.c_str(), -1, nullptr, 0, nullptr, nullptr);

        if (size)
        {
            auto spCharBuffer = std::make_unique<char[]>(size + 1);
            memset(spCharBuffer.get(), 0, size + 1);
            auto result = WideCharToMultiByte(CP_UTF8, 0, kInputString.c_str(), -1, spCharBuffer.get(), size, nullptr, nullptr);

            if (result)
            {
                kOutputString = std::string(spCharBuffer.get());
            }
        }
    }
    else if (std::is_same_v<Dst, std::wstring>)
    {
        auto size = MultiByteToWideChar(CP_UTF8, 0, kInputString.c_str(), kInputString.length(), nullptr, 0);

        if (size)
        {
            auto spCharBuffer = std::make_unique<wchar_t[]>(size + 1);
            memset(spCharBuffer.get(), 0, (size + 1) * sizeof(wchar_t));

            auto result = MultiByteToWideChar(CP_UTF8, 0, kInputString.c_str(), kInputString.length(), spCharBuffer.get(), size);

            if (result)
            {
                kOutputString = std::wstring(spCharBuffer.get());
            }
        }
    }

    return kOutputString;
}