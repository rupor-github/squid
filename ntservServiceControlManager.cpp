/*  
Module : ntservServiceControlManager.cpp
Purpose: Implementation for the class CNTServiceControlManager
Created: PJN / 14-07-1997
History: PJN / 11-01-2006 CNTServiceControlManager::EnumServices method now uses a void* parameter for its 
                          item data instead of a DWORD.
         PJN / 18-05-2006 1. Minor update to rename the local variable of type _NTSERV_SERVICCONTROLMANAGER_DATA 
                          which is used to hold function pointers. Thanks to Frederic Metrich for reporting 
                          this issue.
         PJN / 25-06-2006 1. Combined the functionality of the _NTSERV_SERVICCONTROLMANAGER_DATA class into the 
                          main CNTServiceControlManager class.
                          2. Code now uses newer C++ style casts instead of C style casts.
         PJN / 18-12-2015 1. Added SAL annotations to all the code

Copyright (c) 1996 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////////  Includes  //////////////////////////////////

#include "stdafx.h"
#include "ntservServiceControlManager.h"


////////////////////////////////// Macros / Defines ///////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS


///////////////////////////////// Implementation //////////////////////////////

CNTServiceControlManager::CNTServiceControlManager() : m_hSCM(NULL),
                                                       m_hLock(NULL),
                                                       m_lpfnRegisterServiceCtrlHandlerEx(NULL),
                                                       m_lpfnEnumServicesStatusEx(NULL)
{
  m_hAdvapi32 = GetModuleHandle(_T("ADVAPI32.DLL"));
  if (m_hAdvapi32 != NULL)
  {
  #ifdef _UNICODE
    m_lpfnRegisterServiceCtrlHandlerEx = reinterpret_cast<LPREGISTERSERVICECTRLHANDLEREX>(GetProcAddress(m_hAdvapi32, "RegisterServiceCtrlHandlerExW"));
    m_lpfnEnumServicesStatusEx  = reinterpret_cast<LPENUMSERVICESSTATUSEX>(GetProcAddress(m_hAdvapi32, "EnumServicesStatusExW"));
  #else
    m_lpfnRegisterServiceCtrlHandlerEx = reinterpret_cast<LPREGISTERSERVICECTRLHANDLEREX>(GetProcAddress(m_hAdvapi32, "RegisterServiceCtrlHandlerExA"));
    m_lpfnEnumServicesStatusEx  = reinterpret_cast<LPENUMSERVICESSTATUSEX>(GetProcAddress(m_hAdvapi32, "EnumServicesStatusExA"));
  #endif //#ifdef _UNICODE
  }
}

CNTServiceControlManager::~CNTServiceControlManager()
{
  Unlock();
  Close();
}

CNTServiceControlManager::operator SC_HANDLE() const
{
  return m_hSCM;
}

BOOL CNTServiceControlManager::Attach(_In_opt_ SC_HANDLE hSCM)
{
  if (m_hSCM != hSCM)
    Close();

  m_hSCM = hSCM;
  return TRUE;
}

SC_HANDLE CNTServiceControlManager::Detach()
{
  SC_HANDLE hReturn = m_hSCM;
  m_hSCM = NULL;
  return hReturn;
}

_Must_inspect_result_ _Return_type_success_(return != 0) BOOL CNTServiceControlManager::Open(_In_opt_ LPCTSTR pszMachineName, _In_ DWORD dwDesiredAccess)
{
  Close();
  m_hSCM = OpenSCManager(pszMachineName, SERVICES_ACTIVE_DATABASE, dwDesiredAccess);
  return (m_hSCM != NULL);
}

void CNTServiceControlManager::Close()
{
  if (m_hSCM)
  {
    CloseServiceHandle(m_hSCM);
    m_hSCM = NULL;
  }
}

_Must_inspect_result_ _Return_type_success_(return != 0) BOOL CNTServiceControlManager::QueryLockStatus(_Inout_ LPQUERY_SERVICE_LOCK_STATUS& lpLockStatus) const
{
  //Validate our parameters
  ATLASSUME(m_hSCM != NULL);
  ATLASSERT(lpLockStatus == NULL); //To prevent double overwrites, this function
                                   //asserts if you do not send in a NULL pointer

  DWORD dwBytesNeeded;
  BOOL bSuccess = QueryServiceLockStatus(m_hSCM, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    lpLockStatus = reinterpret_cast<LPQUERY_SERVICE_LOCK_STATUS>(new BYTE[dwBytesNeeded]);
    DWORD dwSize;
    bSuccess = QueryServiceLockStatus(m_hSCM, lpLockStatus, dwBytesNeeded, &dwSize);
  }
  return bSuccess;
}

_Must_inspect_result_ _Return_type_success_(return != 0) BOOL CNTServiceControlManager::EnumServices(_In_ DWORD dwServiceType, _In_ DWORD dwServiceState, _In_opt_ void* pUserData, _In_ ENUM_SERVICES_PROC lpEnumServicesFunc) const
{
  //Validate our parameters
  ATLASSUME(m_hSCM != NULL);

  DWORD dwBytesNeeded;
  DWORD dwServices;
  DWORD dwResumeHandle = 0;
  BOOL bSuccess = EnumServicesStatus(m_hSCM, dwServiceType, dwServiceState, NULL, 0, &dwBytesNeeded, &dwServices, &dwResumeHandle);
  if (!bSuccess && GetLastError() == ERROR_MORE_DATA)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }
  
    LPENUM_SERVICE_STATUS lpServices = reinterpret_cast<LPENUM_SERVICE_STATUS>(lpBuffer.m_pData);
    DWORD dwSize = 0;
    bSuccess = EnumServicesStatus(m_hSCM, dwServiceType, dwServiceState, (LPENUM_SERVICE_STATUS) lpServices, dwBytesNeeded, &dwSize, &dwServices, &dwResumeHandle);
    if (bSuccess)
    {
      BOOL bContinue = TRUE;
      for (DWORD i=0; i<dwServices && bContinue; i++)
        bContinue = lpEnumServicesFunc(pUserData, lpServices[i]);
    }
  }
  return bSuccess;
}

_Must_inspect_result_ _Return_type_success_(return != 0) BOOL CNTServiceControlManager::EnumServices(_In_ DWORD dwServiceType, _In_ DWORD dwServiceState, _In_opt_ LPCTSTR pszGroupName, _In_opt_ void* pUserData, _In_ ENUM_SERVICES_PROC2 lpEnumServicesFunc) const
{
  //Check to see if the function pointer is available
  if (m_lpfnEnumServicesStatusEx == NULL)
  {
    ATLTRACE(_T("CNTServiceControlManager::EnumServices, EnumServicesStatusEx function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  DWORD dwBytesNeeded;
  DWORD dwServices;
  DWORD dwResumeHandle = 0;
  BOOL bSuccess = m_lpfnEnumServicesStatusEx(m_hSCM, SC_ENUM_PROCESS_INFO, dwServiceType, dwServiceState, NULL, 0, &dwBytesNeeded, &dwServices, &dwResumeHandle, pszGroupName);
  if (!bSuccess && GetLastError() == ERROR_MORE_DATA)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }
  
    LPENUM_SERVICE_STATUS_PROCESS lpServices = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(lpBuffer.m_pData);
    DWORD dwSize = 0;
    bSuccess = m_lpfnEnumServicesStatusEx(m_hSCM, SC_ENUM_PROCESS_INFO, dwServiceType, dwServiceState, reinterpret_cast<LPBYTE>(lpServices), dwBytesNeeded, &dwSize, &dwServices, &dwResumeHandle, pszGroupName);
    if (bSuccess)
    {
      BOOL bContinue = TRUE;
      for (DWORD i=0; i<dwServices && bContinue; i++)
        bContinue = lpEnumServicesFunc(pUserData, lpServices[i]);
    }
  }
  return bSuccess;
}

_Must_inspect_result_ _Return_type_success_(return != 0) BOOL CNTServiceControlManager::OpenService(_In_ LPCTSTR lpServiceName, _In_	DWORD dwDesiredAccess, _Out_ CNTScmService& service) const
{
  //Validate our parameters
  ATLASSUME(m_hSCM != NULL);

  SC_HANDLE hService = ::OpenService(m_hSCM, lpServiceName, dwDesiredAccess);
  BOOL bSuccess = (hService != NULL);
  if (bSuccess)
    service.Attach(hService);
  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTServiceControlManager::Lock()
{
  //Validate our parameters
  ATLASSUME(m_hSCM != NULL);

  m_hLock = LockServiceDatabase(m_hSCM);
  return (m_hLock != NULL);
}

_Return_type_success_(return != 0) BOOL CNTServiceControlManager::Unlock()
{
  BOOL bSuccess = TRUE;
  if (m_hLock)
  {
    bSuccess = UnlockServiceDatabase(m_hLock);
    m_hLock = NULL;
  }

  return bSuccess;
}
