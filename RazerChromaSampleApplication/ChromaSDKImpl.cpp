//! \example ChromaSDKImpl.cpp

#include "stdafx.h"
#include <map>
#include <vector>
#include "ChromaSDKImpl.h"

#include <psapi.h>
#include <wintrust.h>
#include <Softpub.h>

#pragma comment (lib, "wintrust")
#pragma comment (lib, "Psapi")
#pragma comment (lib, "crypt32")

CChromaSDKImpl g_ChromaSDKImpl;

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#else
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif

using namespace std;
using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;

typedef RZRESULT (*INIT)(void);
typedef RZRESULT (*UNINIT)(void);
typedef RZRESULT (*CREATEEFFECT)(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATEKEYBOARDEFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATEHEADSETEFFECT)(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATEMOUSEPADEFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATEMOUSEEFFECT)(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATEKEYPADEFFECT)(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*CREATECHROMALINKEFFECT)(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT (*SETEFFECT)(RZEFFECTID EffectId);
typedef RZRESULT (*DELETEEFFECT)(RZEFFECTID EffectId);
typedef RZRESULT (*REGISTEREVENTNOTIFICATION)(HWND hWnd);
typedef RZRESULT (*UNREGISTEREVENTNOTIFICATION)(void);
typedef RZRESULT (*QUERYDEVICE)(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo);

INIT Init = NULL;
UNINIT UnInit = NULL;
CREATEEFFECT CreateEffect = NULL;
CREATEKEYBOARDEFFECT CreateKeyboardEffect = NULL;
CREATEMOUSEEFFECT CreateMouseEffect = NULL;
CREATEHEADSETEFFECT CreateHeadsetEffect = NULL;
CREATEMOUSEPADEFFECT CreateMousematEffect = NULL;
CREATEKEYPADEFFECT CreateKeypadEffect = NULL;
CREATECHROMALINKEFFECT CreateChromaLinkEffect = NULL;
SETEFFECT SetEffect = NULL;
DELETEEFFECT DeleteEffect = NULL;
QUERYDEVICE QueryDevice = NULL;

#define EVENT_NAME  _T("{4784D90A-1179-4F7D-8558-52511D809190}")

#define MAX_EFFECTS     100

typedef struct _EFFECTDATATYPE
{
    LONG numEffects;
    BOOL repeat;
    HANDLE thread;
    struct _EFFECT
    {
        RZEFFECTID id;
        LONG delay;
    } Effect[MAX_EFFECTS];
} EFFECTDATATYPE;

struct GUIDCompare
{
    bool operator()(const GUID & Left, const GUID & Right) const
    {
        return memcmp(&Left , &Right,sizeof(Right)) < 0;
    }
};

std::map<RZEFFECTID, EFFECTDATATYPE, GUIDCompare> g_Effects;

DWORD WINAPI Thread_RenderEffects(LPVOID lpParameter)
{
    RZEFFECTID *pEffectId = (RZEFFECTID*)lpParameter;

    auto iterator = g_Effects.find(*pEffectId);
    if(iterator != g_Effects.end())
    {
        EFFECTDATATYPE *pEffectData = &iterator->second;

        CFrameController FrameControl(30);

        if(pEffectData->repeat == FALSE)
        {
            for(int i=0; i<pEffectData->numEffects; i++)
            {
                FrameControl.Begin();

                SetEffect(pEffectData->Effect[i].id);

                Sleep(pEffectData->Effect[i].delay);

                FrameControl.End();
            }
        }
        else
        {
            while(pEffectData->repeat)
            {
                for(int i=0; i<pEffectData->numEffects; i++)
                {
                    FrameControl.Begin();

                    SetEffect(pEffectData->Effect[i].id);

                    Sleep(pEffectData->Effect[i].delay);

                    FrameControl.End();
                }
            };
        }
    }

    return 0;
}

// Source: https://docs.microsoft.com/en-us/windows/desktop/seccrypto/example-c-program--verifying-the-signature-of-a-pe-file
// Source: https://support.microsoft.com/en-us/help/323809/how-to-get-information-from-authenticode-signed-executables

BOOL IsValidPath(PTCHAR szFileName)
{
    BOOL bResult = FALSE;

    // Get the module name
    TCHAR szModuleName[MAX_PATH] = L"";
    _tcscpy_s(szModuleName, MAX_PATH, szFileName);

    PathStripPath(szModuleName);

    // Verify the path of the module
    // Below are valid paths
    // Windows/System32
    // Windows/SysWOW64
    // Program Files/Razer Chroma SDK/bin
    // Program Files (x86)/Razer Chroma SDK/bin

    DWORD dwLength = 0;
    TCHAR szFileNameExpected[MAX_PATH] = L"";
    TCHAR szPath[MAX_PATH] = L"";

    dwLength = GetEnvironmentVariable(L"SystemRoot",
        szPath,
        MAX_PATH);

    if(dwLength > 0)
    {
        _tcscpy_s(szFileNameExpected, dwLength+1, szPath);

        _tcscat_s(szFileNameExpected, MAX_PATH, L"\\System32\\");
        _tcscat_s(szFileNameExpected, MAX_PATH, szModuleName);

        if(_tcsicmp(szFileNameExpected, szFileName) == 0)
        {
            bResult = TRUE;
        }
    }

    dwLength = GetEnvironmentVariable(L"ProgramFiles",
        szPath,
        MAX_PATH);

    if(dwLength > 0)
    {
        _tcscpy_s(szFileNameExpected, dwLength+1, szPath);

        _tcscat_s(szFileNameExpected, MAX_PATH, L"\\Razer Chroma SDK\\bin\\");
        _tcscat_s(szFileNameExpected, MAX_PATH, szModuleName);

        if(_tcsicmp(szFileNameExpected, szFileName) == 0)
        {
            bResult = TRUE;
        }
    }

    return bResult;
}

BOOL IsFileSignedByRazer(PTCHAR szFileName)
{
    BOOL bResult = FALSE;

    // File is signed, lets check the whos the certificate is issue to.
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    DWORD dwEncoding = 0;
    DWORD dwContentType = 0;
    DWORD dwFormatType = 0;
    LPTSTR szName = NULL;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;
    PCCERT_CONTEXT pCertContext = NULL;

    if(TRUE == CryptQueryObject(CERT_QUERY_OBJECT_FILE,
        szFileName,
        CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
        CERT_QUERY_FORMAT_FLAG_BINARY,
        0,
        &dwEncoding,
        &dwContentType,
        &dwFormatType,
        &hStore,
        &hMsg,
        NULL))
    {
        DWORD dwSignerInfo = 0;

        if(TRUE == CryptMsgGetParam(hMsg,
            CMSG_SIGNER_INFO_PARAM,
            0,
            NULL,
            &dwSignerInfo))
        {
            // Allocate memory for signer information.
            pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);

            if(TRUE == CryptMsgGetParam(hMsg,
                CMSG_SIGNER_INFO_PARAM,
                0,
                (PVOID)pSignerInfo,
                &dwSignerInfo))
            {
                CERT_INFO CertInfo = {};
                CertInfo.Issuer = pSignerInfo->Issuer;
                CertInfo.SerialNumber = pSignerInfo->SerialNumber;

                pCertContext = CertFindCertificateInStore(hStore,
                    (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
                    0,
                    CERT_FIND_SUBJECT_CERT,
                    (PVOID)&CertInfo,
                    NULL);

                if(pCertContext != NULL)
                {
                    DWORD dwData = 0;

                    // Get Subject name size.
                    dwData = CertGetNameString(pCertContext,
                        CERT_NAME_SIMPLE_DISPLAY_TYPE,
                        0,
                        NULL,
                        NULL,
                        0);

                    if(dwData > 1)
                    {
                        // Allocate memory for subject name.
                        szName = (LPTSTR)LocalAlloc(LPTR, dwData * sizeof(TCHAR));

                        // Get subject name.
                        if (CertGetNameString(pCertContext,
                            CERT_NAME_SIMPLE_DISPLAY_TYPE,
                            0,
                            NULL,
                            szName,
                            dwData) > 1)
                        {
                            // Compare the issuer
                            if(_tcsicmp(szName, L"Razer USA Ltd.") == 0)
                            {
                                bResult = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }

    LocalFree(szName);
    LocalFree(pSignerInfo);
    CertFreeCertificateContext(pCertContext);
    CertCloseStore(hStore, 0);
    CryptMsgClose(hMsg);

    return bResult;
}

BOOL IsFileSigned(PTCHAR szFileName)
{
    BOOL bResult = FALSE;
    DWORD dwLastError = 0;

    WINTRUST_FILE_INFO FileData = {};

    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = szFileName;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
    
    1) The certificate used to sign the file chains up to a root 
    certificate located in the trusted root certificate store. This 
    implies that the identity of the publisher has been verified by 
    a certification authority.
    
    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the  
    end entity certificate is stored in the trusted publisher store,  
    implying that the user trusts content from this publisher.
    
    3) The end entity certificate has sufficient permission to sign 
    code, as indicated by the presence of a code signing EKU or no 
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    WINTRUST_DATA WinTrustData = {};

    WinTrustData.cbStruct = sizeof(WinTrustData);
    WinTrustData.pPolicyCallbackData = NULL;
    WinTrustData.pSIPClientData = NULL;
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    WinTrustData.hWVTStateData = NULL;
    WinTrustData.pwszURLReference = NULL;
    WinTrustData.dwUIContext = 0;
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID 
    // and Wintrust_Data.
    LONG lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus) 
    {
    case ERROR_SUCCESS:
        /*
        Signed file:
            - Hash that represents the subject is trusted.

            - Trusted publisher without any verification errors.

            - UI was disabled in dwUIChoice. No publisher or 
                time stamp chain errors.

            - UI was enabled in dwUIChoice and the user clicked 
                "Yes" when asked to install and run the signed 
                subject.
        */
        bResult = IsFileSignedByRazer(szFileName);
        break;
    case TRUST_E_NOSIGNATURE:
        // The file was not signed or had a signature 
        // that was not valid.

        // Get the reason for no signature.
        dwLastError = GetLastError();
        if (TRUST_E_NOSIGNATURE == dwLastError ||
                TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
                TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
        {
            // The file was not signed.
        } 
        else 
        {
            // The signature was not valid or there was an error 
            // opening the file.
        }

        bResult =  FALSE;
        break;
    case TRUST_E_EXPLICIT_DISTRUST:
        // The hash that represents the subject or the publisher 
        // is not allowed by the admin or user.
        bResult =  FALSE;
        break;
    case TRUST_E_SUBJECT_NOT_TRUSTED:
        // The user clicked "No" when asked to install and run.
        bResult =  FALSE;
        break;
    case CRYPT_E_SECURITY_SETTINGS:
        /*
        The hash that represents the subject or the publisher 
        was not explicitly trusted by the admin and the 
        admin policy has disabled user trust. No signature, 
        publisher or time stamp errors.
        */
        bResult =  FALSE;
        break;
    default:
        // The UI was disabled in dwUIChoice or the admin policy 
        // has disabled user trust. lStatus contains the 
        // publisher or time stamp chain error.
        dwLastError = GetLastError();
        bResult =  FALSE;
        break;
    }

    // Any hWVTStateData must be released by a call with close.
    WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    return bResult;
}

BOOL VerifyModule(HMODULE hModule)
{
    TCHAR szFilePath[MAX_PATH];
    if(GetModuleFileNameEx(GetCurrentProcess(),
        hModule,
        szFilePath, 
        MAX_PATH) > 0)
    {
        if((IsValidPath(szFilePath) == TRUE) &&
            (IsFileSigned(szFilePath) == TRUE))
        {
            return TRUE;
        }
    }

    return FALSE;
}

// CChromaSDKImpl

CChromaSDKImpl::CChromaSDKImpl():
    m_hModule(NULL),
    m_hEvent(NULL)
{
}

CChromaSDKImpl::~CChromaSDKImpl()
{
}

void CChromaSDKImpl::Initialize()
{
    if(m_hModule == NULL)
    {
        m_hModule =::LoadLibrary(CHROMASDKDLL);
        if(m_hModule != NULL)
        {
#ifndef _DEBUG
            // Module is not valid!
            if(VerifyModule(m_hModule) == FALSE)
            {
                ::FreeLibrary(m_hModule);
                m_hModule = NULL;
                return;
            }
#endif

            INIT Init = (INIT)::GetProcAddress(m_hModule, "Init");
            if(Init != NULL)
            {
                RZRESULT rzResult = Init();
                if(rzResult == RZRESULT_SUCCESS)
                {
                    CreateEffect = (CREATEEFFECT)::GetProcAddress(m_hModule, "CreateEffect");
                    CreateKeyboardEffect = (CREATEKEYBOARDEFFECT)::GetProcAddress(m_hModule, "CreateKeyboardEffect");
                    CreateMouseEffect = (CREATEMOUSEEFFECT)::GetProcAddress(m_hModule, "CreateMouseEffect");
                    CreateMousematEffect = (CREATEMOUSEPADEFFECT)::GetProcAddress(m_hModule, "CreateMousepadEffect");
                    CreateKeypadEffect = (CREATEKEYPADEFFECT)::GetProcAddress(m_hModule, "CreateKeypadEffect");
                    CreateHeadsetEffect = (CREATEHEADSETEFFECT)::GetProcAddress(m_hModule, "CreateHeadsetEffect");
                    CreateChromaLinkEffect = (CREATECHROMALINKEFFECT)::GetProcAddress(m_hModule, "CreateChromaLinkEffect");
                    SetEffect = (SETEFFECT)GetProcAddress(m_hModule, "SetEffect");
                    DeleteEffect = (DELETEEFFECT)GetProcAddress(m_hModule, "DeleteEffect");
                }
            }
        }
    }

    if(m_hEvent == NULL)
    {
        m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
    }
}

void CChromaSDKImpl::UnInitialize()
{
    // Free memeory
    while(!g_Effects.empty())
    {
        auto iterator = g_Effects.begin();
        for(int i=0; i<iterator->second.numEffects; i++)
        {
            if(DeleteEffect != NULL)
            {
                DeleteEffect(iterator->second.Effect[i].id);
            }
        }

        g_Effects.erase(iterator);
    };

    if(m_hEvent != NULL)
    {
        ::CloseHandle(m_hEvent);
        m_hEvent = NULL;
    }

    if(m_hModule != NULL)
    {
        UNINIT UnInit = (UNINIT)::GetProcAddress(m_hModule, "UnInit");
        if(UnInit != NULL)
        {
            RZRESULT rzResult = UnInit();
            if(rzResult != RZRESULT_SUCCESS)
            {
                // Some error here
            }
        }

        ::FreeLibrary(m_hModule);
        m_hModule = NULL;
    }
}

void CChromaSDKImpl::CreateEffectImpl(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateEffect == NULL) return;

    CreateEffect(DeviceId, Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateKeyboardEffectImpl(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateKeyboardEffect == NULL) return;

    CreateKeyboardEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateMouseEffectImpl(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateMouseEffect == NULL) return;

    CreateMouseEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateMousematEffectImpl(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateMousematEffect == NULL) return;

    CreateMousematEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateKeypadEffectImpl(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateKeypadEffect == NULL) return;

    CreateKeypadEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateHeadsetEffectImpl(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateHeadsetEffect == NULL) return;

    CreateHeadsetEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
    if(CreateChromaLinkEffect == NULL) return;

    CreateChromaLinkEffect(Effect, pParam, pEffectId);
}

void CChromaSDKImpl::SetEffectImpl(RZEFFECTID EffectId)
{
    if(SetEffect == NULL) return;

    auto iterator = g_Effects.find(EffectId);
    if(iterator != g_Effects.end())
    {
        if(iterator->second.repeat == FALSE)
        {
            HANDLE hThread = CreateThread(NULL, 0, Thread_RenderEffects, (LPVOID)&iterator->first, 0, NULL);
            if(hThread != NULL)
            {
                CloseHandle(hThread);
            }
        }
        else
        {
            HANDLE hThread = CreateThread(NULL, 0, Thread_RenderEffects, (LPVOID)&iterator->first, 0, NULL);
            if(hThread != NULL)
            {
                iterator->second.thread = hThread;
            }
        }
    }
    else
    {
        if(SetEffect == NULL) return;

        SetEffect(EffectId);
    }
}

void CChromaSDKImpl::DeleteEffectImpl(RZEFFECTID EffectId)
{
    if(DeleteEffect == NULL) return;

    auto iterator = g_Effects.find(EffectId);
    if(iterator != g_Effects.end())
    {
        EFFECTDATATYPE EffectData = iterator->second;
        for(int i=0; i<EffectData.numEffects; i++)
        {
            DeleteEffect(EffectData.Effect[i].id);
        }

        g_Effects.erase(iterator);
    }
    else
    {
        if(DeleteEffect == NULL) return;

        DeleteEffect(EffectId);
    }
}

void CChromaSDKImpl::StopEffectImpl(RZEFFECTID EffectId)
{
    auto iterator = g_Effects.find(EffectId);
    if(iterator != g_Effects.end())
    {
        if((iterator->second.repeat == TRUE) &&
            (iterator->second.thread != NULL))
        {
            iterator->second.repeat = FALSE;

            CloseHandle(iterator->second.thread);

            iterator->second.thread = NULL;
        }
    }
}

void CChromaSDKImpl::CreateEffectGroup(RZEFFECTID *pGroupEffectId, BOOL Repeat)
{
    RZEFFECTID EffectId = GUID_NULL;
    if(SUCCEEDED(::CoCreateGuid(&EffectId)))
    {
        EFFECTDATATYPE EffectData = {};

        EffectData.numEffects = 0;
        EffectData.repeat = Repeat;

        g_Effects.insert(make_pair(EffectId, EffectData));

        *pGroupEffectId = EffectId;
    }
}

void CChromaSDKImpl::AddToGroup(RZEFFECTID GroupEffectId, RZEFFECTID EffectId, LONG DelayMS)
{
    auto iterator = g_Effects.find(GroupEffectId);
    if(iterator != g_Effects.end())
    {
        LONG lIndex = iterator->second.numEffects;

        iterator->second.Effect[lIndex].id = EffectId;
        iterator->second.Effect[lIndex].delay = DelayMS;

        iterator->second.numEffects++;
    }
}
