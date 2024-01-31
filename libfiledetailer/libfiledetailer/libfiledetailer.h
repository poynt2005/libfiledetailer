#ifndef __LIB_FILE_DETAILER_H__
#define __LIB_FILE_DETAILER_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __BUILD_DYN_LIB
#define FD_API __declspec(dllexport) 
#else
#define FD_API
#endif


typedef uint64_t FDHandle;

typedef enum __e_fd_error_code
{
    FD_CallSuccess = 0,
    FD_CallError,
    FD_NotCreated,
} EFDErrorCode;

typedef struct __fd_details 
{
    char* szPropKey;
    char* szPropValue;
    struct __fd_details* next;
} CFDDetails;

#ifdef __cplusplus
extern "C" 
{
#endif

    FD_API FDHandle FileDetailer_Create();
    FD_API void FileDetailer_Free(const FDHandle hFd);
    FD_API EFDErrorCode FileDetailer_GetFileDetailW(const FDHandle hFd, const wchar_t* wszFilePath);
    FD_API EFDErrorCode FileDetailer_GetFileDetailA(const FDHandle hFd, const char* szFilePath);
    FD_API EFDErrorCode FileDetailer_GetLastErrorString(const FDHandle hFd, char** pszErrorString);
    FD_API EFDErrorCode FileDetailer_GetLastDetails(const FDHandle hFd, CFDDetails** ppkFdDetails);

    FD_API void FileDetailer_FreeDetailsBuffer(CFDDetails** ppkFdDetails);
    FD_API void FileDetailer_FreeStringBuffer(char** pBuffer);

#ifdef __cplusplus
}
#endif

#endif