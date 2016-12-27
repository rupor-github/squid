/*  
Module : ntservEventLogSource.cpp
Purpose: Implementation for the class CNTEventLogSource
Created: PJN / 14-07-1997
History: PJN / 23-07-2005 1. Provided an overriden version of Report which has 3 string parameters
                          2. GetStringArrayFromRegistry and SetStringArrayIntoRegistry methods
                          are now public as they are helpful to have available externally
         PJN / 11-01-2006 1. Addition of a default constructor for CNTEventLogSource 
                          2. Addition of CNTEventLogSource::SetServerName, CNTEventLogSource::GetServerName,
                          CNTEventLogSource::SetSourceName and CNTEventLogSource::GetSourceName methods
                          3. Addition of a DWORD* parameter to the CNTEventLogSource::GetStringArrayFromRegistry
                          and CNTEventLogSource::SetStringArrayFromRegistry functions. This allow the details
                          of the last error to be returned to client code.
         PJN / 03-06-2006 1. If CNTEventLogSource::Uninstall fails to remove the HKLM\System\CurrentControlSet\Services\EventLog\"Display Name"
                          registry key, then it aborts before doing any other work. Thanks to Matthias Miller
                          for reporting this issue.
         PJN / 13-06-2006 1. Addition of a logname parameter to the CNTEventLogSource constructor to support user defined event logs
                          instead of logging to the standard "Application" event log. Please note that the code added only enables the CNTService 
                          framework to use non "Application" event logs, but it does not contain code to create a custom event log. You will need 
                          to handle that in your application's install. For further information on doing this, please see 
                          http://msdn.microsoft.com/library/default.asp?url=/library/en-us/eventlog/base/eventlog_key.asp which contains details
                          on the various registry values which you need to create. Thanks to Jan Bartels for this addition.
                          2. Also optimized the construction of member variables in the CNTEventLogSource constructors
         PJN / 24-06-2006 1. Updated code to compile cleanly using VC 2005.
         PJN / 25-06-2006 1. Code now uses newer C++ style casts instead of C style casts.
         PJN / 01-08-2006 1. Fix for two buffer size calculations using the new _tcscpy_s functions, one in CNTEventLogSource::SetStringArrayIntoRegistry and the other in
                          CNTService::GetDependencies. Thanks to DmityShm on the codeproject forums for reporting this issue.
         PJN / 21-09-2006 1. Minor update to use %X instead of %x in various CNTEventLogSource::Report calls.
         PJN / 13-11-2007 1. Minor update to display HRESULT's correctly.
         PJN / 01-08-2010 1. Fixed a minor issue in CNTEventLogSource::Report with the use of printf format identifiers
                          2. Reworked all the internal registry management code to use ATL::CRegKey
                          3. CNTEventLogSource::Install method now supports CategoryCount, CategoryMessageFile and ParameterMessageFile settings for event sources
         PJN / 10-11-2012 1. Fixed a bug in CNTEventLogSource::GetStringArrayFromRegistry where it could not handle empty MULTI_SZ strings. Thanks to Rolf Schatten
                          for reporting this bug.
                          2. CNTEventLogSource::Install now calls CRegKey::Create instead of CRegKey::Open when installing the 
                          HKLM\SYSTEM\CurrentControlSet\Services\EventLog\"LogName" registry key. This ensures that event log sources which are created with custom
                          LogName's will be installed correctly. Thanks to Rolf Schatten for reporting this bug.
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


///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "ntservEventLogSource.h"
#include "ntserv_msg.h"


///////////////////////////////// Macros //////////////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS


///////////////////////////////// Includes ////////////////////////////////////

CNTEventLogSource::CNTEventLogSource() : m_hEventSource(NULL),
                                         m_sLogName(_T("Application"))
{
}

CNTEventLogSource::CNTEventLogSource(_In_opt_z_ LPCTSTR lpUNCServerName, _In_opt_z_ LPCTSTR lpSourceName, _In_opt_z_ LPCTSTR lpLogName) : m_hEventSource(NULL),
                                                                                                                                          m_sServerName((lpUNCServerName == NULL) ? _T("") : lpUNCServerName),
                                                                                                                                          m_sSourceName((lpSourceName == NULL) ? _T("") : lpSourceName),
                                                                                                                                          m_sLogName((lpLogName == NULL) ? _T("") : lpLogName)

{
}

CNTEventLogSource::CNTEventLogSource(_In_opt_z_ LPCTSTR lpUNCServerName, _In_opt_z_ LPCTSTR lpSourceName) : m_hEventSource(NULL),
                                                                                                            m_sServerName((lpUNCServerName == NULL) ? _T("") : lpUNCServerName),
                                                                                                            m_sSourceName((lpSourceName == NULL) ? _T("") : lpSourceName),
                                                                                                            m_sLogName(_T("Application"))
{
}

CNTEventLogSource::~CNTEventLogSource()
{
  Deregister();
}

CNTEventLogSource::operator HANDLE() const
{
  return m_hEventSource;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Attach(_In_opt_ HANDLE hEventSource)
{
  if (m_hEventSource != hEventSource)
    Deregister();

  m_hEventSource = hEventSource;
  return TRUE;
}

HANDLE CNTEventLogSource::Detach()
{
  HANDLE hReturn = m_hEventSource;
  m_hEventSource = NULL;
  return hReturn;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Register(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpSourceName)
{
  Deregister();
  m_hEventSource = RegisterEventSource(lpUNCServerName, lpSourceName);
  return (m_hEventSource != NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ WORD wCategory, _In_ DWORD dwEventID, _In_opt_ PSID lpUserSid,
                                                                  _In_ WORD wNumStrings, _In_ DWORD dwDataSize, _In_reads_opt_(wNumStrings) LPCTSTR* lpStrings, _In_reads_bytes_opt_(dwDataSize) LPVOID lpRawData)
{
  ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csReport, true);

  ATLASSERT(m_hEventSource == NULL);
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (!Register(m_sServerName, m_sSourceName))
#else
  if (!Register(m_sServerName.c_str(), m_sSourceName.c_str()))
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
    return FALSE;
  ATLASSUME(m_hEventSource != NULL);

  //Call the SDK version of the function
  BOOL bSuccess = ReportEvent(m_hEventSource, wType,	wCategory, dwEventID, lpUserSid,
                              wNumStrings, dwDataSize, lpStrings, lpRawData);
  Deregister();  

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID)
{
  return Report(wType, 0, dwEventID, NULL, 0, 0, NULL, NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_opt_z_ LPCTSTR lpszString)
{
  return Report(wType, CNTS_MSG_SERVICE_FULLSTRING, lpszString);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString)
{
  ATLASSERT(lpszString != NULL);
  return Report(wType, 0, dwEventID, NULL, 1, 0, &lpszString, NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2)
{
  ATLASSERT(lpszString1 != NULL);
  ATLASSERT(lpszString2 != NULL);
  LPCTSTR lpStrings[2];
  lpStrings[0] = lpszString1;
  lpStrings[1] = lpszString2;
  return Report(wType, 0, dwEventID, NULL, 2, 0, lpStrings, NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2, _In_ DWORD dwCode, _In_ BOOL bReportAsHex)
{
  ATLASSERT(lpszString1 != NULL);
  ATLASSERT(lpszString2 != NULL);
  LPCTSTR lpStrings[3];
  lpStrings[0] = lpszString1;
  lpStrings[1] = lpszString2;
  TCHAR szError[32];
  if (bReportAsHex)
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%08X"), dwCode);
  else
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%u"), dwCode);
  lpStrings[2] = szError;
  return Report(wType, 0, dwEventID, NULL, 3, 0, lpStrings, NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2, _In_opt_z_ LPCTSTR lpszString3)
{
  ATLASSERT(lpszString1 != NULL);
  ATLASSERT(lpszString2 != NULL);
  ATLASSERT(lpszString3 != NULL);
  LPCTSTR lpStrings[3];
  lpStrings[0] = lpszString1;
  lpStrings[1] = lpszString2;
  lpStrings[2] = lpszString3;
  return Report(wType, 0, dwEventID, NULL, 3, 0, lpStrings, NULL);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_ DWORD dwCode, _In_ BOOL bReportAsHex)
{
  TCHAR szError[32];
  if (bReportAsHex)
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%08X"), dwCode);
  else
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%u"), dwCode);
  return Report(wType, dwEventID, szError);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString, _In_ DWORD dwCode, _In_ BOOL bReportAsHex)
{
  TCHAR szError[32];
  if (bReportAsHex)
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%08X"), dwCode);
  else
    _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%u"), dwCode);
  return Report(wType, dwEventID, lpszString, szError);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Deregister()
{
  BOOL bSuccess = TRUE;
  if (m_hEventSource != NULL)
  {
    bSuccess = DeregisterEventSource(m_hEventSource);
    m_hEventSource = NULL;
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Install(_In_opt_z_ LPCTSTR lpszSourceName, _In_opt_z_ LPCTSTR lpszEventMessageFile, _In_opt_z_ LPCTSTR pszEventCategoryMessageFile, _In_opt_z_ LPCTSTR pszEventParameterMessageFile, _In_ DWORD dwTypesSupported, _In_ DWORD dwCategoryCount)
{
  //Just delegate the work to the other version of "Install"
  return Install(_T("Application"), lpszSourceName, lpszEventMessageFile, pszEventCategoryMessageFile, pszEventParameterMessageFile, dwTypesSupported, dwCategoryCount);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Install(_In_opt_z_ LPCTSTR lpszLogName, _In_opt_z_ LPCTSTR lpszSourceName, _In_opt_z_ LPCTSTR lpszEventMessageFile, _In_opt_z_ LPCTSTR pszEventCategoryMessageFile, _In_opt_z_ LPCTSTR pszEventParameterMessageFile, _In_ DWORD dwTypesSupported, _In_ DWORD dwCategoryCount)
{
  //Validate our parameters
  ATLASSUME(lpszLogName != NULL);
  ATLASSERT(_tcslen(lpszLogName));
  ATLASSUME(lpszSourceName != NULL);
  ATLASSERT(_tcslen(lpszSourceName));
  ATLASSUME(lpszEventMessageFile != NULL);

  //What will be the return value from this function, assume the worst
  BOOL bSuccess = FALSE;

  //Make the necessary updates to the registry
  TCHAR szKey[4096];
  _stprintf_s(szKey, sizeof(szKey)/sizeof(TCHAR), _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s"), lpszLogName);
  ATL::CRegKey appKey;
  if (appKey.Create(HKEY_LOCAL_MACHINE, szKey) == ERROR_SUCCESS)
  {
    ATL::CRegKey sourceKey;
    if (sourceKey.Create(appKey, lpszSourceName, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL) == ERROR_SUCCESS)
    {
      //Write the EventMessageFile string value
      bSuccess = sourceKey.SetStringValue(_T("EventMessageFile"), lpszEventMessageFile) == ERROR_SUCCESS;

      //Write the TypesSupported value
      bSuccess = bSuccess && sourceKey.SetDWORDValue(_T("TypesSupported"), dwTypesSupported) == ERROR_SUCCESS;

      //Write the CategoryCount value if required
      if (dwCategoryCount && bSuccess)
        bSuccess = sourceKey.SetDWORDValue(_T("CategoryCount"), dwCategoryCount) == ERROR_SUCCESS;

      //Write the CategoryMessageFile string if required
      if (pszEventCategoryMessageFile && _tcslen(pszEventCategoryMessageFile) && bSuccess)
        bSuccess = sourceKey.SetStringValue(_T("CategoryMessageFile"), pszEventCategoryMessageFile) == ERROR_SUCCESS;

      //Write the ParameterMessageFile string if required
      if (pszEventParameterMessageFile && _tcslen(pszEventParameterMessageFile) && bSuccess)
        bSuccess = sourceKey.SetStringValue(_T("ParameterMessageFile"), pszEventParameterMessageFile) == ERROR_SUCCESS;

      //Update the sources registry key so that the event viewer can filter on the events which we write to the event log
      if (bSuccess)
      {
        CNTServiceStringArray sources;
        if (GetStringArrayFromRegistry(appKey, _T("Sources"), sources))
        {
          //If our name is not in the array then add it
          BOOL bFoundMyself = FALSE;
        #ifdef CNTSERVICE_MFC_EXTENSIONS
          for (int i=0; i<sources.GetSize() && !bFoundMyself; i++)
            bFoundMyself = (sources.GetAt(i) == lpszSourceName);
        #else
          bFoundMyself = (std::find(sources.begin(), sources.end(), lpszSourceName) != sources.end());
        #endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
          if (!bFoundMyself)
          {
          #ifdef CNTSERVICE_MFC_EXTENSIONS
            sources.Add(lpszSourceName);
          #else
            sources.push_back(lpszSourceName);
          #endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
            SetStringArrayIntoRegistry(appKey, _T("Sources"), sources);
          }
        }
      }
    }
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Uninstall(_In_opt_z_ LPCTSTR lpSourceName)
{
  //Just delegate the work to the other version of "Install"
  return Uninstall(_T("Application"), lpSourceName);
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::Uninstall(_In_opt_z_ LPCTSTR lpszLogName, _In_opt_z_ LPCTSTR lpszSourceName)
{
  //Validate our parameters
  ATLASSUME(lpszLogName != NULL);
  ATLASSERT(_tcslen(lpszLogName));
  ATLASSUME(lpszSourceName != NULL);
  ATLASSERT(_tcslen(lpszSourceName));

  //Remove the settings from the registry
  TCHAR szSubKey[4096];
  _stprintf_s(szSubKey, sizeof(szSubKey)/sizeof(TCHAR), _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s"), lpszLogName, lpszSourceName);
  long nSuccess = RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
  if (nSuccess != ERROR_SUCCESS) //If we cannot delete this registry key, then abort this function before we go any further
  {
    SetLastError(nSuccess); //Make the last error value available to our callers 
    return FALSE;
  }

  //Remove ourself from the "Sources" registry key
  _stprintf_s(szSubKey, sizeof(szSubKey)/sizeof(TCHAR), _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s"), lpszLogName);
  ATL::CRegKey appKey;
  if (appKey.Open(HKEY_LOCAL_MACHINE, szSubKey, KEY_WRITE | KEY_READ) == ERROR_SUCCESS)
  {
    CNTServiceStringArray sources;
    if (GetStringArrayFromRegistry(appKey, _T("Sources"), sources))
    {
      //If our name is in the array then remove it
      BOOL bFoundMyself = FALSE;

    #ifdef CNTSERVICE_MFC_EXTENSIONS
      for (int i=0; i<sources.GetSize() && !bFoundMyself; i++)
      {
        bFoundMyself = (sources.GetAt(i) == lpszSourceName);
        if (bFoundMyself)
        {
          sources.RemoveAt(i);
        }
      }
    #else
      CNTServiceStringArray::iterator iterFind = std::find(sources.begin(), sources.end(), lpszSourceName);
      bFoundMyself = (iterFind != sources.end());
      if (bFoundMyself)
        sources.erase(iterFind);
    #endif //#ifdef CNTSERVICE_MFC_EXTENSIONS

      if (bFoundMyself)
        SetStringArrayIntoRegistry(appKey, _T("Sources"), sources);
    }
  }

  return TRUE;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::GetStringArrayFromRegistry(_In_ ATL::CRegKey& key, _In_opt_z_ LPCTSTR lpszEntry, _Out_ CNTServiceStringArray& array, _Inout_opt_ DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszEntry != NULL);

  //What will be the return value from this function, assume the worst
  BOOL bSuccess = FALSE;

  //Initialize the output parameters to default values
  if (pLastError != NULL)
    *pLastError = ERROR_SUCCESS;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  array.RemoveAll();
#else
  array.clear();
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS

  DWORD dwType = 0;
  ULONG nBytes = 0;
  LONG lResult = key.QueryValue(lpszEntry, &dwType, NULL, &nBytes);
  if (lResult == ERROR_SUCCESS)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<TCHAR> lpBuffer;
    ULONG nChars = nBytes / sizeof(TCHAR);
    if (nChars < 2) //Ensure we can handle an empty MULTI_SZ string
      nChars = 2;
    if (!lpBuffer.Allocate(nChars))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      if (pLastError != NULL)
        *pLastError = ERROR_OUTOFMEMORY;
      return FALSE;
    }

    lResult = key.QueryMultiStringValue(lpszEntry, lpBuffer, &nChars);      
    if (lResult == ERROR_SUCCESS)
    {
      LPTSTR lpszStrings = lpBuffer.m_pData;
      while (lpszStrings[0] != 0)
      {
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        array.Add(lpszStrings);
      #else
        array.push_back(lpszStrings);
      #endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
        lpszStrings += (_tcslen(lpszStrings) + 1);
      }

      bSuccess = TRUE;
    }
    else
    {
      if (pLastError != NULL)
        *pLastError = lResult;
    }
  }
  else
  {
    if (pLastError != NULL)
      *pLastError = lResult;
  }

  return bSuccess;
}

_Return_type_success_(return != 0) BOOL CNTEventLogSource::SetStringArrayIntoRegistry(_In_ ATL::CRegKey& key, _In_opt_z_ LPCTSTR lpszEntry, _In_ const CNTServiceStringArray& array, _Inout_opt_ DWORD* pLastError)
{   
  //Validate our input parameters
  ATLASSERT(lpszEntry != NULL);

  //Initialize the output parameter to default value
  if (pLastError != NULL)
    *pLastError = ERROR_SUCCESS;

  //Work out the size of the buffer we will need
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nSize = 0;
  INT_PTR nStrings = array.GetSize();
  for (INT_PTR i=0; i<nStrings; i++)
    nSize += array.GetAt(i).GetLength() + 1; //1 extra for each NULL terminator
#else
  CNTServiceStringArray::size_type nSize = 0;
  CNTServiceStringArray::size_type nStrings = array.size();
  for (CNTServiceStringArray::size_type i=0; i<nStrings; i++)
    nSize += array[i].length() + 1; //1 extra for each NULL terminator
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS

  //Need one second NULL for the double NULL at the end
  nSize++;

  //Allocate some memory for the API
  ATL::CHeapPtr<TCHAR> lpBuffer;
  if (!lpBuffer.Allocate(nSize))
  {
    SetLastError(ERROR_OUTOFMEMORY);
    if (pLastError != NULL)
      *pLastError = ERROR_OUTOFMEMORY;
    return FALSE;
  }

  //Now copy the strings into the buffer
  LPTSTR lpszString = lpBuffer.m_pData;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nCurOffset = 0;
  for (INT_PTR i=0; i<nStrings; i++)
#else
  CNTServiceStringArray::size_type nCurOffset = 0;
  for (CNTServiceStringArray::size_type i=0; i<nStrings; i++)
#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
  {
    const CNTServiceString& sText = array[i];
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    int nCurrentStringLength = sText.GetLength();
    _tcscpy_s(&lpszString[nCurOffset], nCurrentStringLength+1, sText);
  #else
    CNTServiceString::size_type nCurrentStringLength = sText.length();
    _tcscpy_s(&lpszString[nCurOffset], nCurrentStringLength+1, sText.c_str());
  #endif //#ifdef CNTSERVICE_MFC_EXTENSIONS
    nCurOffset += (nCurrentStringLength + 1);
  }
  //Don't forgot to doubly NULL terminate
  lpszString[nCurOffset] = _T('\0');

  //Finally write it into the registry
  LONG lResult = key.SetMultiStringValue(lpszEntry, lpBuffer);
  BOOL bSuccess = (lResult == ERROR_SUCCESS);
  if (!bSuccess && (pLastError != NULL))
    *pLastError = lResult;

  return bSuccess;
}
