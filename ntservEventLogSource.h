/*
Module : ntservEventLogSource.h
Purpose: Defines the interface for the class CNTEventLogSource.
Created: PJN / 14-07-1998

Copyright (c) 1997 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////////// Macros / Defines ///////////////////////////////

#pragma once

#ifndef __NTSERVEVENTLOGSOURCE_H__
#define __NTSERVEVENTLOGSOURCE_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//An encapsulation of the APIs used to register, unregister, write, install and uninstall Event log entries 
//i.e. the server side to the Event log APIs
class CNTSERVICE_EXT_CLASS CNTEventLogSource
{
public:
//Constructors / Destructors
  CNTEventLogSource();
  CNTEventLogSource(_In_opt_z_ LPCTSTR lpUNCServerName, _In_opt_z_ LPCTSTR lpSourceName, _In_opt_z_ LPCTSTR lpLogName);
  CNTEventLogSource(_In_opt_z_ LPCTSTR lpUNCServerName, _In_opt_z_ LPCTSTR lpSourceName);
  ~CNTEventLogSource();

//Accessors / Mutators
  void             SetServerName(_In_opt_z_ LPCTSTR pszServerName) { m_sServerName = pszServerName; };
  CNTServiceString GetServerName() const { return m_sServerName; };
  void             SetSourceName(_In_opt_z_ LPCTSTR pszSourceName) { m_sSourceName = pszSourceName; };
  CNTServiceString GetSourceName() const { return m_sSourceName; };
  void             SetLogName(_In_opt_z_ LPCTSTR pszLogName) { m_sLogName = pszLogName; };
  CNTServiceString GetLogName() const { return m_sLogName; };

//Methods
  operator HANDLE() const;
  _Return_type_success_(return != 0) BOOL Attach(_In_opt_ HANDLE hEventSource);
  HANDLE                                  Detach();
  _Return_type_success_(return != 0) BOOL Register(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpSourceName);
  _Return_type_success_(return != 0) BOOL Deregister();
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ WORD wCategory, _In_ DWORD dwEventID, _In_opt_ PSID lpUserSid,
                                                 _In_ WORD wNumStrings, _In_ DWORD dwDataSize, _In_reads_opt_(wNumStrings) LPCTSTR* lpStrings, _In_reads_bytes_opt_(dwDataSize) LPVOID lpRawData);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_opt_z_ LPCTSTR lpszString);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2, _In_opt_z_ LPCTSTR lpszString3);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString1, _In_opt_z_ LPCTSTR lpszString2, _In_ DWORD dwCode, _In_ BOOL bReportAsHex = FALSE);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_opt_z_ LPCTSTR lpszString, _In_ DWORD dwCode, _In_ BOOL bReportAsHex = FALSE);
  _Return_type_success_(return != 0) BOOL Report(_In_ WORD wType, _In_ DWORD dwEventID, _In_ DWORD dwCode, _In_ BOOL bReportAsHex = FALSE);

  static _Return_type_success_(return != 0) BOOL Install(_In_opt_z_ LPCTSTR lpszSourceName, _In_opt_z_ LPCTSTR lpszEventMessageFile, _In_opt_z_ LPCTSTR pszEventCategoryMessageFile, _In_opt_z_ LPCTSTR pszEventParameterMessageFile, _In_ DWORD dwTypesSupported, _In_ DWORD dwCategoryCount);
  static _Return_type_success_(return != 0) BOOL Install(_In_opt_z_ LPCTSTR lpszLogName, _In_opt_z_ LPCTSTR lpszSourceName, _In_opt_z_ LPCTSTR lpszEventMessageFile, _In_opt_z_ LPCTSTR pszEventCategoryMessageFile, _In_opt_z_ LPCTSTR pszEventParameterMessageFile, _In_ DWORD dwTypesSupported, _In_ DWORD dwCategoryCount);
  static _Return_type_success_(return != 0) BOOL Uninstall(_In_opt_z_ LPCTSTR lpSourceName);
  static _Return_type_success_(return != 0) BOOL Uninstall(_In_opt_z_ LPCTSTR lpszLogName, _In_opt_z_ LPCTSTR lpszSourceName);
  static _Return_type_success_(return != 0) BOOL GetStringArrayFromRegistry(_In_ ATL::CRegKey& key, _In_opt_z_ LPCTSTR lpszEntry, _Out_ CNTServiceStringArray& array, _Inout_opt_ DWORD* pLastError = NULL);
  static _Return_type_success_(return != 0) BOOL SetStringArrayIntoRegistry(_In_ ATL::CRegKey& key, _In_opt_z_ LPCTSTR lpszEntry, _In_ const CNTServiceStringArray& array, _Inout_opt_ DWORD* pLastError = NULL);
  
protected:
  HANDLE                       m_hEventSource;
  CNTServiceString             m_sServerName;
  CNTServiceString             m_sSourceName;
  CNTServiceString             m_sLogName;
  ATL::CComAutoCriticalSection m_csReport; //Critical section to protect multiple threads calling Report at the one time

  friend class CNTService;
};

#endif //#ifndef __NTSERVEVENTLOGSOURCE_H__
