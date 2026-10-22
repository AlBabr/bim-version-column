#pragma once

#include <windows.h>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>
#include <string>

// Уникальный CLSID для Property Handler.
// ВАЖНО: замените на свой GUID, сгенерированный через guidgen.exe или PowerShell [guid]::NewGuid().
// Значение должно совпадать в: BimPropertyHandler.cpp (DllGetClassObject), BimPropertyHandler.def, RegisterBimHandler.bat
// {12345678-1234-1234-1234-1234567890AB}
static const CLSID CLSID_BimPropertyHandler =
    { 0x16a5be61c, 0x19b6, 0x4653, { 0x8f, 0xea, 0xfb, 0x7a, 0x37, 0xa5, 0x51, 0xef } };

// FMTID и PID для свойства BIM Version.
// Должны совпадать с BimVersion.propdesc.
// {A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
static const GUID GUID_BIM_VERSION =
    { 0xA1B2C3D4, 0xE5F6, 0x7890, { 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90 } };
static const PROPERTYKEY PKEY_BimVersion = { GUID_BIM_VERSION, 100 };

class BimPropertyHandler : public IInitializeWithFile, public IPropertyStore
{
public:
    BimPropertyHandler();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IInitializeWithFile
    IFACEMETHODIMP Initialize(LPCWSTR pszFilePath, DWORD grfMode) override;

    // IPropertyStore
    IFACEMETHODIMP GetCount(DWORD* cProps) override;
    IFACEMETHODIMP GetAt(DWORD iProp, PROPERTYKEY* pkey) override;
    IFACEMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override;
    IFACEMETHODIMP SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override;
    IFACEMETHODIMP Commit() override;

private:
    ~BimPropertyHandler();

    LONG m_refCount;
    std::wstring m_version;
    bool m_initialized;
};

class BimPropertyHandlerFactory : public IClassFactory
{
public:
    BimPropertyHandlerFactory();

    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override;
    IFACEMETHODIMP LockServer(BOOL fLock) override;

private:
    ~BimPropertyHandlerFactory();
    LONG m_refCount;
};

// Глобальные счётчики для DllCanUnloadNow / LockServer
extern LONG g_cDllRef;
extern LONG g_cLockCount;