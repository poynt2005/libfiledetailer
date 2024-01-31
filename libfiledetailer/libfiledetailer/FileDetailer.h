#pragma once

#include <Windows.h>
#include <shobjidl_core.h>
#include <atlbase.h>
#include <pathcch.h>
#include <shldisp.h>

#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <vector>


enum class EFileDetailerErrorCode
{
    NoError = 0,
    TargetFileNotExists,
    ComNotInitialize,
    ComInitializeFailed,
    ComGetClsIDFailed,
    ComCreateIDispatchFailed,
    ComCallNamespaceFailed,
    ComEnumFolderItemsFailed,
    ComEnumFolderItemsCountFailed,
    ComEmptyFolderItemsFailed,
    ComTargetFilePathNotFoundFailed,
};



class CFileDetailer 
{
public:
    CFileDetailer();
    ~CFileDetailer();

    bool GetFileDetail(const std::string& strFilePath);
    bool GetFileDetail(const std::wstring& wstrFilePath);
    EFileDetailerErrorCode GetLastError() const { return m_eLastErrorCode; }
    bool GetIsComInitializied() const { return m_bIsComInitialized; }
    std::vector<std::pair<std::string, std::string>> GetLastDetails() const { return std::vector<std::pair<std::string, std::string>>(m_kLastDetails.begin(), m_kLastDetails.end()); }
    
    static std::string ErrorCodeToString(const EFileDetailerErrorCode eCode);

private:
    bool m_bIsComInitialized = false;
    CComPtr<IShellDispatch> m_spDispatch = nullptr;
    std::vector<std::pair<std::string, std::string>> m_kLastDetails;

    EFileDetailerErrorCode m_eLastErrorCode = EFileDetailerErrorCode::NoError;

    template <typename Dst, typename Src>
    static Dst ConvertString(const Src& inputString);
};