/*
Module : ntservEventLog.h
Purpose: Defines the interface for the class CNTEventLog. 
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


/////////////////////////////// Macros / Defines //////////////////////////////

#pragma once

#ifndef __NTSERVEVENTLOG_H__
#define __NTSERVEVENTLOG_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservEventLogRecord.h"
#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//An encapsulation of the client side to the NT event log APIs
class CNTSERVICE_EXT_CLASS CNTEventLog
{
public:
//Constructors / Destructors
  CNTEventLog();
  ~CNTEventLog();

//Methods
  operator HANDLE() const;
  _Return_type_success_(return != 0) BOOL Attach(_In_opt_ HANDLE hEventLog);
  HANDLE                                  Detach();
  _Return_type_success_(return != 0) BOOL Open(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpSourceName);
  _Return_type_success_(return != 0) BOOL OpenBackup(_In_opt_ LPCTSTR lpUNCServerName, _In_ LPCTSTR lpFileName);
  _Return_type_success_(return != 0) BOOL OpenApplication(_In_opt_ LPCTSTR lpUNCServerName);
  _Return_type_success_(return != 0) BOOL OpenSystem(_In_opt_ LPCTSTR lpUNCServerName);
  _Return_type_success_(return != 0) BOOL OpenSecurity(_In_opt_ LPCTSTR lpUNCServerName);
  _Return_type_success_(return != 0) BOOL Close();
  _Return_type_success_(return != 0) BOOL Backup(_In_ LPCTSTR lpBackupFileName) const;
  _Return_type_success_(return != 0) BOOL Clear(_In_opt_ LPCTSTR lpBackupFileName) const;
  _Return_type_success_(return != 0) BOOL GetNumberOfRecords(_Out_ DWORD& dwNumberOfRecords) const;
  _Return_type_success_(return != 0) BOOL GetOldestRecord(_Out_ DWORD& dwOldestRecord) const;
  _Return_type_success_(return != 0) BOOL NotifyChange(_In_ HANDLE hEvent) const;
  _Return_type_success_(return != 0) BOOL ReadNext(_Out_ CEventLogRecord& record) const;
  _Return_type_success_(return != 0) BOOL ReadPrev(_Out_ CEventLogRecord& record) const;
  _Return_type_success_(return != 0) BOOL GetFullInformation(_Out_ DWORD& dwFull) const;

protected:
  HANDLE m_hEventLog;
};

#endif //#ifndef __NTSERVEVENTLOG_H__
