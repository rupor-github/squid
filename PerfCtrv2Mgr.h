/*
Module : PerfCtrv2Mgr.h
Purpose: Defines the interface for a set of C++ wrapper classes for the Perf Counter v2 framework 
         available in Windows Vista and later
Created: PJN / 11-10-2012
History: PJN / 02-01-2016 1. Updated copyright details
                          2. Added SAL annotations to all the code
                          3. Removed unnecessary CStringW instance from CPerfProvider::QueryInstance method
                          4. CPerfCounterSet::CreateInstance now uses a PCWSTR parameter instead of a 
                          LPCTSTR parameter. This avoids the need to use of CStringW local variable
                          5. Updated the parameter values names for the classes to be consistent with the 
                          Windows 10 SDK perflib.h header.
                          6. Updated the code to compile without taking a dependency on MFC.
         PJN / 10-02-2016 1. Updated the code to use LoadLibrary rather than relying on client applications
                          already having a implicit dependency on ADVAPI32.dll. This bug / issue was 
                          spotted when you compile the sample app in release mode which ships with the 
                          wrapper classes.

Copyright (c) 2012 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////// Macros / Defines ///////////////////////////////////

#pragma once

#ifndef __PERFCTRV2MGR_H__
#define __PERFCTRV2MGR_H__

#ifndef _Inout_updates_bytes_
#define _Inout_updates_bytes_(size)
#endif //#ifndef _Inout_updates_bytes_


///////////////////////// Includes ////////////////////////////////////////////

#ifndef _PERFLIB_H_
#pragma message("To avoid this message, please put perflib.h in your pre compiled header (normally stdafx.h)")
#include <perflib.h>
#endif //#ifndef _PERFLIB_H_


///////////////////////// Classes /////////////////////////////////////////////

//Class wrapper for a perf provider
class CPerfProvider
{
public:
//Constructors / Destructors
  CPerfProvider() : m_hProvider(NULL),
                    m_pfnPerfStartProvider(NULL),
                    m_pfnPerfStartProviderEx(NULL),
                    m_pfnPerfStopProvider(NULL),
                    m_pfnPerfCreateInstance(NULL),
                    m_pfnPerfDeleteInstance(NULL),
                    m_pfnPerfQueryInstance(NULL),
                    m_pfnPerfSetCounterSetInfo(NULL),
                    m_pfnPerfIncrementULongCounterValue(NULL),
                    m_pfnPerfIncrementULongLongCounterValue(NULL),
                    m_pfnPerfSetULongCounterValue(NULL),
                    m_pfnPerfSetULongLongCounterValue(NULL),
                    m_pfnPerfDecrementULongCounterValue(NULL),
                    m_pfnPerfDecrementULongLongCounterValue(NULL),
                    m_pfnPerfSetCounterRefValue(NULL)
  {
    //Get the function pointers we need
    m_hAdvapi = LoadLibraryFromSystem32(_T("ADVAPI32.DLL"));
    if (m_hAdvapi != NULL)
    {
      m_pfnPerfStartProvider = reinterpret_cast<PERFSTARTPROVIDER*>(GetProcAddress(m_hAdvapi, "PerfStartProvider"));
      m_pfnPerfStartProviderEx = reinterpret_cast<PERFSTARTPROVIDEREX*>(GetProcAddress(m_hAdvapi, "PerfStartProviderEx"));
      m_pfnPerfStopProvider = reinterpret_cast<PERFSTOPPROVIDER*>(GetProcAddress(m_hAdvapi, "PerfStopProvider"));
      m_pfnPerfCreateInstance = reinterpret_cast<PERFCREATEINSTANCE*>(GetProcAddress(m_hAdvapi, "PerfCreateInstance"));
      m_pfnPerfDeleteInstance = reinterpret_cast<PERFDELETEINSTANCE*>(GetProcAddress(m_hAdvapi, "PerfDeleteInstance"));
      m_pfnPerfQueryInstance = reinterpret_cast<PERFQUERYINSTANCE*>(GetProcAddress(m_hAdvapi, "PerfQueryInstance"));
      m_pfnPerfSetCounterSetInfo = reinterpret_cast<PERFSETCOUNTERSETINFO*>(GetProcAddress(m_hAdvapi, "PerfSetCounterSetInfo"));
      m_pfnPerfIncrementULongCounterValue = reinterpret_cast<PERFINCREMENTULONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfIncrementULongCounterValue"));
      m_pfnPerfIncrementULongLongCounterValue = reinterpret_cast<PERFINCREMENTULONGLONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfIncrementULongLongCounterValue"));
      m_pfnPerfSetULongCounterValue = reinterpret_cast<PERFSETULONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfSetULongCounterValue"));
      m_pfnPerfSetULongLongCounterValue = reinterpret_cast<PERFSETULONGLONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfSetULongLongCounterValue"));
      m_pfnPerfDecrementULongCounterValue = reinterpret_cast<PERFDECREMENTULONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfDecrementULongCounterValue"));
      m_pfnPerfDecrementULongLongCounterValue = reinterpret_cast<PERFDECREMENTULONGLONGCOUNTERVALUE*>(GetProcAddress(m_hAdvapi, "PerfDecrementULongLongCounterValue"));
      m_pfnPerfSetCounterRefValue = reinterpret_cast<PERFSETCOUNTERREFVALUE*>(GetProcAddress(m_hAdvapi, "PerfSetCounterRefValue"));
    }
  }

  ~CPerfProvider()
  {
    Stop();
    if (m_hAdvapi != NULL)
    {
      FreeLibrary(m_hAdvapi);
      m_hAdvapi = NULL;
    }
  }

//Methods
  ULONG Start(_In_ LPGUID Guid, _In_opt_ PERFLIBREQUEST ControlCallback = NULL)
  {
    //Validate our parameters
    ATLASSERT(m_hProvider == NULL);

    //Call through the function pointer
    if ((m_pfnPerfStartProvider != NULL) && (m_pfnPerfStopProvider != NULL))
      return m_pfnPerfStartProvider(Guid, ControlCallback, &m_hProvider);
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG StartEx(_In_ LPGUID Guid, _In_opt_ PPERF_PROVIDER_CONTEXT ProviderContext)
  {
    //Validate our parameters
    ATLASSERT(m_hProvider == NULL);

    //Call through the function pointer
    if ((m_pfnPerfStartProviderEx != NULL) && (m_pfnPerfStopProvider != NULL))
      return m_pfnPerfStartProviderEx(Guid, ProviderContext, &m_hProvider);
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG StartEx(_In_ LPGUID Guid, _In_opt_ PERFLIBREQUEST NotificationCallback = NULL, _In_opt_ PERF_MEM_ALLOC MemoryAllocationFunction = NULL,
                _In_opt_ PERF_MEM_FREE MemoryFreeFunction = NULL, _In_opt_ PVOID MemoryFunctionsContext = NULL)
  {
    //Setup an appropriate PERF_PROVIDER_CONTEXT
    PERF_PROVIDER_CONTEXT ProviderContext;
    memset(&ProviderContext, 0, sizeof(ProviderContext));
    ProviderContext.ContextSize = sizeof(ProviderContext);
    ProviderContext.ControlCallback = NotificationCallback;
    ProviderContext.MemAllocRoutine = MemoryAllocationFunction;
    ProviderContext.MemFreeRoutine = MemoryFreeFunction;
    ProviderContext.pMemContext = MemoryFunctionsContext;

    //Delegate to the other version of "StartEx"
    return StartEx(Guid, &ProviderContext);
  }

  ULONG Stop()
  {
    ULONG nReturn = ERROR_INVALID_HANDLE;
    if (m_hProvider)
    {
      //Validate our parameters
      ATLASSUME(m_pfnPerfStopProvider != NULL);

      //Call through the function pointer
      nReturn = m_pfnPerfStopProvider(m_hProvider);
      m_hProvider = NULL;
    }
    
    return nReturn;
  }  

  PPERF_COUNTERSET_INSTANCE QueryInstance(_In_ LPCGUID Guid, _In_ PCWSTR Name, _In_ ULONG Id)
  {
    //Validate our parameters
    ATLASSERT(m_hProvider != NULL);

    //Call through the function pointer
    if (m_pfnPerfQueryInstance != NULL)
      return m_pfnPerfQueryInstance(m_hProvider, Guid, Name, Id);
    else
    {
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
      return NULL;
    }
  }
  
  ULONG SetCounterSetInfo(_Inout_updates_bytes_(TemplateSize) PPERF_COUNTERSET_INFO Template, _In_ ULONG TemplateSize)
  {
    //Validate our parameters
    ATLASSERT(m_hProvider != NULL);

    //Call through the function pointer
    if (m_pfnPerfSetCounterSetInfo != NULL)
      return m_pfnPerfSetCounterSetInfo(m_hProvider, Template, TemplateSize);
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

protected:
  HMODULE LoadLibraryFromSystem32(LPCTSTR lpFileName)
  {
    //Get the Windows System32 directory
    TCHAR szFullPath[_MAX_PATH];
    szFullPath[0] = _T('\0');
    if (GetSystemDirectory(szFullPath, _countof(szFullPath)) == 0)
      return NULL;

    //Setup the full path and delegate to LoadLibrary    
    #pragma warning(suppress: 6102) //There is a bug with the SAL annotation of GetSystemDirectory in the Windows 8.1 SDK
    _tcscat_s(szFullPath, _countof(szFullPath), _T("\\"));
    _tcscat_s(szFullPath, _countof(szFullPath), lpFileName);
    return LoadLibrary(szFullPath);
  }

//Typedefs
  typedef ULONG (WINAPI PERFSTARTPROVIDER)(LPGUID, PERFLIBREQUEST, HANDLE*);
  typedef ULONG (WINAPI PERFSTARTPROVIDEREX)(LPGUID, PPERF_PROVIDER_CONTEXT, PHANDLE);  
  typedef ULONG (WINAPI PERFSTOPPROVIDER)(HANDLE);
  typedef PPERF_COUNTERSET_INSTANCE (WINAPI PERFCREATEINSTANCE)(HANDLE, LPCGUID, PCWSTR, ULONG);
  typedef ULONG (WINAPI PERFDELETEINSTANCE)(HANDLE, PPERF_COUNTERSET_INSTANCE);
  typedef PPERF_COUNTERSET_INSTANCE (WINAPI PERFQUERYINSTANCE)(HANDLE, LPCGUID, PCWSTR, ULONG);
  typedef ULONG (WINAPI PERFSETCOUNTERSETINFO)(HANDLE, PPERF_COUNTERSET_INFO, ULONG);
  typedef ULONG (WINAPI PERFINCREMENTULONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONG);
  typedef ULONG (WINAPI PERFINCREMENTULONGLONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONGLONG);
  typedef ULONG (WINAPI PERFSETULONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONG);
  typedef ULONG (WINAPI PERFSETULONGLONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONGLONG);
  typedef ULONG (WINAPI PERFDECREMENTULONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONG);
  typedef ULONG (WINAPI PERFDECREMENTULONGLONGCOUNTERVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, ULONGLONG);
  typedef ULONG (WINAPI PERFSETCOUNTERREFVALUE)(HANDLE, PPERF_COUNTERSET_INSTANCE, ULONG, PVOID);

//Member variables
  HMODULE                             m_hAdvapi;                //The Advapi32 dll handle
  HANDLE                              m_hProvider;              //The provider handle we are wrapping
  PERFSTARTPROVIDER*                  m_pfnPerfStartProvider;   //The various function pointers
  PERFSTARTPROVIDEREX*                m_pfnPerfStartProviderEx;
  PERFSTOPPROVIDER*                   m_pfnPerfStopProvider;
  PERFCREATEINSTANCE*                 m_pfnPerfCreateInstance;
  PERFDELETEINSTANCE*                 m_pfnPerfDeleteInstance;
  PERFQUERYINSTANCE*                  m_pfnPerfQueryInstance;
  PERFSETCOUNTERSETINFO*              m_pfnPerfSetCounterSetInfo;
  PERFINCREMENTULONGCOUNTERVALUE*     m_pfnPerfIncrementULongCounterValue;
  PERFINCREMENTULONGLONGCOUNTERVALUE* m_pfnPerfIncrementULongLongCounterValue;
  PERFSETULONGCOUNTERVALUE*           m_pfnPerfSetULongCounterValue;
  PERFSETULONGLONGCOUNTERVALUE*       m_pfnPerfSetULongLongCounterValue;
  PERFDECREMENTULONGCOUNTERVALUE*     m_pfnPerfDecrementULongCounterValue;
  PERFDECREMENTULONGLONGCOUNTERVALUE* m_pfnPerfDecrementULongLongCounterValue;
  PERFSETCOUNTERREFVALUE*             m_pfnPerfSetCounterRefValue;

  friend class CPerfCounterSet; //Allows this clas to use the internal cached function pointers we maintain
};


//Class wrapper for a perf counter set instance
class CPerfCounterSet
{
public:
//Constructors / Destructors
  CPerfCounterSet() : m_pInstance(NULL), 
                      m_pProvider(NULL)
  {
  }

  ~CPerfCounterSet()
  {
    //Free up the instance if valid
    if (m_pInstance != NULL)
      DeleteInstance();
  }

//Methods  
  void Initialize(_In_ CPerfProvider* pProvider)
  {
    //Validate our parameters
    ATLASSUME(m_pProvider == NULL);
    m_pProvider = pProvider;
    ATLASSERT(m_pProvider->m_hProvider != NULL);
  }

  BOOL Initialized() const
  {
    return (m_pProvider != NULL);
  }

  ULONG CreateInstance(_In_ LPGUID Guid, _In_ PCWSTR Name, _In_ ULONG Id)
  {
    //Validate our parameters
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);
    ATLASSERT(m_pInstance == NULL);

    //What will be the return value from this function
    ULONG nReturn = ERROR_SUCCESS;

    //Call through the function pointer
    if ((m_pProvider->m_pfnPerfCreateInstance != NULL) && (m_pProvider->m_pfnPerfDeleteInstance != NULL))
    {
      m_pInstance =  m_pProvider->m_pfnPerfCreateInstance(m_pProvider->m_hProvider, Guid, Name, Id);
      if (m_pInstance == NULL)
        nReturn = GetLastError();
    }
    else
      nReturn = ERROR_CALL_NOT_IMPLEMENTED;

    return nReturn;
  }

  ULONG DeleteInstance()
  {
    //Validate our parameters
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);
    ATLASSERT(m_pInstance != NULL);

    //What will be the return value from this function
    ULONG nReturn = ERROR_SUCCESS;

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfDeleteInstance != NULL)
    {
      nReturn = m_pProvider->m_pfnPerfDeleteInstance(m_pProvider->m_hProvider, m_pInstance);
      m_pInstance = NULL;
    }
    else
      nReturn = ERROR_CALL_NOT_IMPLEMENTED;

    return nReturn;
  }

  ULONG IncrementL(_In_ ULONG CounterId, _In_ ULONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfIncrementULongCounterValue != NULL)
      return m_pProvider->m_pfnPerfIncrementULongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG IncrementLL(_In_ ULONG CounterId, _In_ ULONGLONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfIncrementULongLongCounterValue != NULL)
      return m_pProvider->m_pfnPerfIncrementULongLongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG SetL(_In_ ULONG CounterId, _In_ ULONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfSetULongCounterValue != NULL)
      return m_pProvider->m_pfnPerfSetULongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG SetLL(_In_ ULONG CounterId, _In_ ULONGLONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfSetULongCounterValue != NULL)
      return m_pProvider->m_pfnPerfSetULongLongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG DecrementL(_In_ ULONG CounterId, _In_ ULONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfDecrementULongCounterValue != NULL)
      return m_pProvider->m_pfnPerfDecrementULongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG DecrementLL(_In_ ULONG CounterId, _In_ ULONGLONG Value)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfDecrementULongLongCounterValue != NULL)
      return m_pProvider->m_pfnPerfDecrementULongLongCounterValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Value);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

  ULONG SetRef(_In_ ULONG CounterId, _In_ PVOID Address)
  {
    //Validate our parameters
    ATLASSERT(m_pInstance != NULL);  
    ATLASSUME(m_pProvider != NULL);
    ATLASSERT(m_pProvider->m_hProvider != NULL);

    //Call through the function pointer
    if (m_pProvider->m_pfnPerfSetCounterRefValue != NULL)
      return m_pProvider->m_pfnPerfSetCounterRefValue(m_pProvider->m_hProvider, m_pInstance, CounterId, Address);    
    else
      return ERROR_CALL_NOT_IMPLEMENTED;
  }

protected:
//Member variables
  PPERF_COUNTERSET_INSTANCE m_pInstance;
  CPerfProvider*            m_pProvider;
};

#endif //#ifndef __PERFCTRV2MGR_H__