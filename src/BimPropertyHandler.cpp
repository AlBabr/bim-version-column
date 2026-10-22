#include "BimPropertyHandler.h"
#include "BimParser.h"
#include <new>
#include <fstream>
#include <mutex>
#include <string>

LONG g_cDllRef = 0;
LONG g_cLockCount = 0;
static HINSTANCE g_hInst = nullptr;

// ==================== ЛОГИРОВАНИЕ ====================

std::mutex g_logMutex;

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], sizeNeeded, nullptr, nullptr);
    return strTo;
}

void DebugLog(const std::wstring& msg) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::ofstream log("C:\\BimHandlerDebug.log", std::ios::app);
    if (log) {
        log << WStringToString(msg) << "\n";
    }
}

// ==================== BimPropertyHandler ====================

BimPropertyHandler::BimPropertyHandler()
    : m_refCount(1), m_initialized(false)
{
    InterlockedIncrement(&g_cDllRef);
}

BimPropertyHandler::~BimPropertyHandler()
{
    InterlockedDecrement(&g_cDllRef);
}

IFACEMETHODIMP BimPropertyHandler::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IInitializeWithFile)
        *ppv = static_cast<IInitializeWithFile*>(this);
    else if (riid == IID_IPropertyStore)
        *ppv = static_cast<IPropertyStore*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) BimPropertyHandler::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) BimPropertyHandler::Release()
{
    ULONG c = InterlockedDecrement(&m_refCount);
    if (c == 0) delete this;
    return c;
}

IFACEMETHODIMP BimPropertyHandler::Initialize(LPCWSTR pszFilePath, DWORD /*grfMode*/)
{
    if (!pszFilePath) return E_INVALIDARG;
    if (m_initialized) return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);

    try {
        DebugLog(L"--- Initialize called for: " + std::wstring(pszFilePath));
        
        m_version = GetBimVersion(pszFilePath);
        
        if (m_version.empty()) {
            DebugLog(L"[EMPTY] " + std::wstring(pszFilePath));
        } else {
            DebugLog(L"[OK]    " + std::wstring(pszFilePath) + L" -> " + m_version);
        }
    }
    catch (const std::exception& e) {
        std::string whatStr(e.what());
        std::wstring wideWhat(whatStr.begin(), whatStr.end());
        DebugLog(L"[EXCEPTION] " + std::wstring(pszFilePath) + L" : " + wideWhat);
        m_version = L"";
    }
    catch (...) {
        DebugLog(L"[UNKNOWN EXCEPTION] " + std::wstring(pszFilePath));
        m_version = L"";
    }

    m_initialized = true;
    return S_OK;
}

IFACEMETHODIMP BimPropertyHandler::GetCount(DWORD* cProps)
{
    if (!cProps) return E_POINTER;
    *cProps = 1;
    return S_OK;
}

IFACEMETHODIMP BimPropertyHandler::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    if (!pkey) return E_POINTER;
    if (iProp != 0) return E_INVALIDARG;
    *pkey = PKEY_BimVersion;
    return S_OK;
}

IFACEMETHODIMP BimPropertyHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if (!pv) return E_POINTER;
    PropVariantInit(pv);

    if (IsEqualPropertyKey(key, PKEY_BimVersion))
    {
        return InitPropVariantFromString(m_version.c_str(), pv);
    }
    return S_FALSE;
}

IFACEMETHODIMP BimPropertyHandler::SetValue(REFPROPERTYKEY, REFPROPVARIANT)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP BimPropertyHandler::Commit()
{
    return S_OK;
}

// ==================== BimPropertyHandlerFactory ====================

BimPropertyHandlerFactory::BimPropertyHandlerFactory()
    : m_refCount(1)
{
    InterlockedIncrement(&g_cDllRef);
}

BimPropertyHandlerFactory::~BimPropertyHandlerFactory()
{
    InterlockedDecrement(&g_cDllRef);
}

IFACEMETHODIMP BimPropertyHandlerFactory::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory)
        *ppv = static_cast<IClassFactory*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) BimPropertyHandlerFactory::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) BimPropertyHandlerFactory::Release()
{
    ULONG c = InterlockedDecrement(&m_refCount);
    if (c == 0) delete this;
    return c;
}

IFACEMETHODIMP BimPropertyHandlerFactory::CreateInstance(
    IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
    if (pUnkOuter) return CLASS_E_NOAGGREGATION;
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    BimPropertyHandler* pHandler = new (std::nothrow) BimPropertyHandler();
    if (!pHandler) return E_OUTOFMEMORY;

    HRESULT hr = pHandler->QueryInterface(riid, ppv);
    pHandler->Release();
    return hr;
}

IFACEMETHODIMP BimPropertyHandlerFactory::LockServer(BOOL fLock)
{
    if (fLock) InterlockedIncrement(&g_cLockCount);
    else       InterlockedDecrement(&g_cLockCount);
    return S_OK;
}

// ==================== DllMain / COM exports ====================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (rclsid != CLSID_BimPropertyHandler)
        return CLASS_E_CLASSNOTAVAILABLE;

    BimPropertyHandlerFactory* pFactory = new (std::nothrow) BimPropertyHandlerFactory();
    if (!pFactory) return E_OUTOFMEMORY;

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllCanUnloadNow()
{
    return (g_cDllRef == 0 && g_cLockCount == 0) ? S_OK : S_FALSE;
}

// ==================== Registration ====================

static LONG RegSetKeyValueString(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, LPCWSTR data)
{
    HKEY hSub = nullptr;
    LONG result = RegCreateKeyExW(hKey, subKey, 0, nullptr, 0, KEY_WRITE, nullptr, &hSub, nullptr);
    if (result != ERROR_SUCCESS) return result;

    result = RegSetValueExW(hSub, valueName, 0, REG_SZ,
                            reinterpret_cast<const BYTE*>(data),
                            static_cast<DWORD>((wcslen(data) + 1) * sizeof(wchar_t)));
    RegCloseKey(hSub);
    return result;
}

STDAPI DllRegisterServer()
{
    wchar_t dllPath[MAX_PATH];
    if (!GetModuleFileNameW(g_hInst, dllPath, MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());

    wchar_t clsidKey[128];
    swprintf_s(clsidKey, L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        CLSID_BimPropertyHandler.Data1, CLSID_BimPropertyHandler.Data2,
        CLSID_BimPropertyHandler.Data3,
        CLSID_BimPropertyHandler.Data4[0], CLSID_BimPropertyHandler.Data4[1],
        CLSID_BimPropertyHandler.Data4[2], CLSID_BimPropertyHandler.Data4[3],
        CLSID_BimPropertyHandler.Data4[4], CLSID_BimPropertyHandler.Data4[5],
        CLSID_BimPropertyHandler.Data4[6], CLSID_BimPropertyHandler.Data4[7]);

    LONG result = RegSetKeyValueString(HKEY_CLASSES_ROOT, clsidKey, nullptr, L"BIM Version Property Handler");
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32(result);

    wchar_t inprocKey[160];
    swprintf_s(inprocKey, L"%s\\InProcServer32", clsidKey);

    result = RegSetKeyValueString(HKEY_CLASSES_ROOT, inprocKey, nullptr, dllPath);
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32(result);

    result = RegSetKeyValueString(HKEY_CLASSES_ROOT, inprocKey, L"ThreadingModel", L"Apartment");
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32(result);

    // Обязательный флаг для IInitializeWithFile
    HKEY hClsidKey = nullptr;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, clsidKey, 0, KEY_WRITE, &hClsidKey) == ERROR_SUCCESS) {
        DWORD dwDisable = 1;
        RegSetValueExW(hClsidKey, L"DisableProcessIsolation", 0, REG_DWORD,
                       reinterpret_cast<const BYTE*>(&dwDisable), sizeof(dwDisable));
        RegCloseKey(hClsidKey);
    }

    return S_OK;
}

STDAPI DllUnregisterServer()
{
    wchar_t clsidKey[128];
    swprintf_s(clsidKey, L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        CLSID_BimPropertyHandler.Data1, CLSID_BimPropertyHandler.Data2,
        CLSID_BimPropertyHandler.Data3,
        CLSID_BimPropertyHandler.Data4[0], CLSID_BimPropertyHandler.Data4[1],
        CLSID_BimPropertyHandler.Data4[2], CLSID_BimPropertyHandler.Data4[3],
        CLSID_BimPropertyHandler.Data4[4], CLSID_BimPropertyHandler.Data4[5],
        CLSID_BimPropertyHandler.Data4[6], CLSID_BimPropertyHandler.Data4[7]);

    RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidKey);
    return S_OK;
}