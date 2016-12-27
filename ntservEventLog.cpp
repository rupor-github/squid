/*  
Module : ntservEventLog.cpp
Purpose: Implementation for the class CNTEventLog
Created: PJN / 14-07-1997
History: PJN / 25-06-2006 1. Code now uses newer C++ style casts instead of C style casts.
         PJN / 02-02-2007 1. Optimized CNTEventLog constructor code.
         PJN / 01-08-2010 1. Added support for GetEventLogInformation
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


///////////////////////////////// Includes  ///////////////////////////////////

#include "stdafx.h"
#include "ntservEventlog.h"


///////////////////////////////// Macros //////////////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS


///////////////////////////////// Implementation //////////////////////////////

CNTEventLog::CNTEventLog() : m_hEventLog(NULL)
{
}

CNTEventLog::~CNTEventLog()
{
  Close();
}

CNTEventLog::operator HANDLE() const
{
  return m_hEventLog;
}

_Return_type_success_(return != 0) BOOL CNTEventLog::Attach(_In_opt_ HANDLE hEventLog)
{
  if (m_hEventLog != hEventLog)
    Close();

  m_hEventLog = hEventLog;
  return TRUE;
}

HANDLE CNTEventLog::Detach()
{
  HANDLE hReturn = m_hEventLog;
  m_hEventLog = NULL;
  return hReturn;
}

_Return_type_success_(return != 0) BOOL CNTEventLog::Open(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpSourceName)
{
  Close();
  m_hEventLog = OpenEventLog(lpUNCServerName, lpSourceName);
  return (m_hEventLog != NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::OpenApplication(_In_opt_ LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("Application"));
}

_Return_type_success_(return != 0) BOOL CNTEventLog::OpenSystem(_In_opt_ LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("System"));
}

_Return_type_success_(return != 0) BOOL CNTEventLog::OpenSecurity(_In_opt_ LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("Security"));
}

_Return_type_success_(return != 0) BOOL CNTEventLog::OpenBackup(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpFileName)
{
  Close();
  m_hEventLog = OpenBackupEventLog(lpUNCServerName, lpFileName);
  return (m_hEventLog != NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::Close()
{
  BOOL bSuccess = TRUE;
  if (m_hEventLog != NULL)
  {
    bSuccess = CloseEventLog(m_hEventLog);
    m_hEventLog = NULL;
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLog::Backup(_In_ LPCTSTR lpBackupFileName) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return BackupEventLog(m_hEventLog, lpBackupFileName);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::Clear(_In_opt_ LPCTSTR lpBackupFileName) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return ClearEventLog(m_hEventLog, lpBackupFileName);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::GetNumberOfRecords(_Out_ DWORD& dwNumberOfRecords) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return GetNumberOfEventLogRecords(m_hEventLog, &dwNumberOfRecords);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::GetOldestRecord(_Out_ DWORD& dwOldestRecord) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return GetOldestEventLogRecord(m_hEventLog, &dwOldestRecord);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::NotifyChange(_In_ HANDLE hEvent) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return NotifyChangeEventLog(m_hEventLog, hEvent);
}

_Return_type_success_(return != 0) BOOL CNTEventLog::ReadNext(_Out_ CEventLogRecord& record) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  DWORD dwBytesRead;
  DWORD dwBytesNeeded;
  EVENTLOGRECORD el;
  BOOL bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, &el, sizeof(EVENTLOGRECORD), &dwBytesRead, &dwBytesNeeded);
  if (bSuccess)
    record = CEventLogRecord(&el);
  else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, lpBuffer, dwBytesNeeded, &dwBytesRead, &dwBytesNeeded);
    if (bSuccess)
      record = CEventLogRecord(reinterpret_cast<EVENTLOGRECORD*>(lpBuffer.m_pData));
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLog::ReadPrev(_Out_ CEventLogRecord& record) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  DWORD dwBytesRead;
  DWORD dwBytesNeeded;
  EVENTLOGRECORD el;
  BOOL bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, &el, sizeof(EVENTLOGRECORD), &dwBytesRead, &dwBytesNeeded);
  if (bSuccess)
    record = CEventLogRecord(&el);
  else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }
  
    bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, lpBuffer, dwBytesNeeded, &dwBytesRead, &dwBytesNeeded);
    if (bSuccess)
      record = CEventLogRecord(reinterpret_cast<EVENTLOGRECORD*>(lpBuffer.m_pData));
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLog::GetFullInformation(_Out_ DWORD& dwFull) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  //Call through the function pointer
  EVENTLOG_FULL_INFORMATION efi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = GetEventLogInformation(m_hEventLog, EVENTLOG_FULL_INFO, &efi, sizeof(efi), &dwBytesNeeded);
  if (bSuccess)
    dwFull = efi.dwFull;

  return bSuccess;
}
