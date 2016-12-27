/*
Module : ntservDefines.h
Purpose: Various defines which the CNTService class framework which requires
Created: PJN / 08-09-2003

Copyright (c) 1997 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////////// Macros / Defines /////////////////////////////////////

#pragma once

#ifndef __NTSERVDEFINES_H__
#define __NTSERVDEFINES_H__


//////////////////////// Enums / Defines //////////////////////////////////////

#ifndef CNTSERVICE_EXT_CLASS
#define CNTSERVICE_EXT_CLASS
#endif //#ifndef CNTSERVICE_EXT_CLASS

#ifndef __ATLBASE_H__
#pragma message("To avoid this message, please put atlbase.h in your pre compiled header (normally stdafx.h)")
#include <atlbase.h>
#endif //#ifndef __ATLBASE_H__

#ifdef CNTSERVICE_MFC_EXTENSIONS

#ifndef __AFXMT_H__
#include <afxmt.h>
#pragma message("To avoid this message, please put afxmt.h in your pre compiled header (normally stdafx.h)")
#endif //#ifndef __AFXMT_H__

#define CNTServiceString CString
#define CNTServiceByteArray CByteArray
#define CNTServiceStringArray CStringArray

#else

#ifndef _STRING_
#pragma message("To avoid this message, please put string in your pre compiled header (normally stdafx.h)")
#include <string>
#endif //#ifndef _STRING_

#ifndef _VECTOR_
#pragma message("To avoid this message, please put vector in your pre compiled header (normally stdafx.h)")
#include <vector>
#endif //#ifndef _VECTOR_

#ifndef _ALGORITHM_
#pragma message("To avoid this message, please put algorithm in your pre compiled header (normally stdafx.h)")
#include <algorithm>
#endif //#ifndef _ALGORITHM_

#ifdef _UNICODE
  #define CNTServiceString std::wstring
  #define CNTServiceStringArray std::vector<std::wstring>
#else
  #define CNTServiceString std::string
  #define CNTServiceStringArray std::vector<std::string>
#endif //#ifdef _UNICODE

#define CNTServiceByteArray std::vector<BYTE>

#endif //#ifdef CNTSERVICE_MFC_EXTENSIONS


//Various defines from WinSvc.h to allow code to compile without the need for the latest Windows SDK
__if_not_exists (PFN_SC_NOTIFY_CALLBACK)
{
  typedef VOID (CALLBACK* PFN_SC_NOTIFY_CALLBACK ) (IN PVOID pParameter);
}
__if_not_exists (SERVICE_NOTIFY_1)
{
  typedef struct _SERVICE_NOTIFY_1 
  {
    DWORD                   dwVersion;
    PFN_SC_NOTIFY_CALLBACK  pfnNotifyCallback;
    PVOID                   pContext;
    DWORD                   dwNotificationStatus;
    SERVICE_STATUS_PROCESS  ServiceStatus;
  } SERVICE_NOTIFY_1, *PSERVICE_NOTIFY_1;
}

__if_not_exists (SERVICE_NOTIFY_2A)
{
  typedef struct _SERVICE_NOTIFY_2A 
  {
    DWORD                   dwVersion;
    PFN_SC_NOTIFY_CALLBACK  pfnNotifyCallback;
    PVOID                   pContext;
    DWORD                   dwNotificationStatus;
    SERVICE_STATUS_PROCESS  ServiceStatus;
    DWORD                   dwNotificationTriggered;
    LPSTR                   pszServiceNames;
  } SERVICE_NOTIFY_2A, *PSERVICE_NOTIFY_2A;
}

__if_not_exists (SERVICE_NOTIFY_2W)
{
  typedef struct _SERVICE_NOTIFY_2W 
  {
    DWORD                   dwVersion;
    PFN_SC_NOTIFY_CALLBACK  pfnNotifyCallback;
    PVOID                   pContext;
    DWORD                   dwNotificationStatus;
    SERVICE_STATUS_PROCESS  ServiceStatus;
    DWORD                   dwNotificationTriggered;
    LPWSTR                  pszServiceNames;
  } SERVICE_NOTIFY_2W, *PSERVICE_NOTIFY_2W;
}

#ifdef _UNICODE
__if_not_exists (SERVICE_NOTIFY_2)
{
  typedef SERVICE_NOTIFY_2W SERVICE_NOTIFY_2;
}
__if_not_exists (PSERVICE_NOTIFY_2)
{
  typedef PSERVICE_NOTIFY_2W PSERVICE_NOTIFY_2;
}
#else
__if_not_exists (SERVICE_NOTIFY_2)
{
  typedef SERVICE_NOTIFY_2A SERVICE_NOTIFY_2;
}
__if_not_exists (PSERVICE_NOTIFY_2)
{
  typedef PSERVICE_NOTIFY_2A PSERVICE_NOTIFY_2;
}
#endif //#ifdef _UNICODE
__if_not_exists (SERVICE_NOTIFYA)
{
  typedef SERVICE_NOTIFY_2A SERVICE_NOTIFYA, *PSERVICE_NOTIFYA;
}
__if_not_exists (SERVICE_NOTIFYW)
{
  typedef SERVICE_NOTIFY_2W SERVICE_NOTIFYW, *PSERVICE_NOTIFYW;
}
#ifdef _UNICODE
__if_not_exists (SERVICE_NOTIFY)
{
  typedef SERVICE_NOTIFYW SERVICE_NOTIFY;
}
__if_not_exists (PSERVICE_NOTIFY)
{
  typedef PSERVICE_NOTIFYW PSERVICE_NOTIFY;
}
#else
__if_not_exists (SERVICE_NOTIFY)
{
  typedef SERVICE_NOTIFYA SERVICE_NOTIFY;
}
__if_not_exists (PSERVICE_NOTIFY)
{
  typedef PSERVICE_NOTIFYA PSERVICE_NOTIFY;
}
#endif //#ifdef _UNICODE

__if_not_exists (SERVICE_TRIGGER_SPECIFIC_DATA_ITEM)
{
  typedef struct _SERVICE_TRIGGER_SPECIFIC_DATA_ITEM
  {
    DWORD   dwDataType; // Data type -- one of SERVICE_TRIGGER_DATA_TYPE_* constants
  #ifdef __midl
    [range(0, 1024)]
  #endif //#ifdef __midl
    DWORD   cbData;     //Size of trigger specific data
  #ifdef __midl
    [size_is(cbData)]
  #endif //#ifdef __midl
    PBYTE   pData;      //Trigger specific data
  } SERVICE_TRIGGER_SPECIFIC_DATA_ITEM, *PSERVICE_TRIGGER_SPECIFIC_DATA_ITEM;
}

__if_not_exists (SERVICE_TRIGGER)
{
  typedef struct _SERVICE_TRIGGER
  {
    DWORD                       dwTriggerType;              //One of SERVICE_TRIGGER_TYPE_* constants
    DWORD                       dwAction;                   //One of SERVICE_TRIGGER_ACTION_* constants
    GUID    *                   pTriggerSubtype;            //Provider GUID if the trigger type is SERVICE_TRIGGER_TYPE_CUSTOM
                                                            //Device class interface GUID if the trigger type is
                                                            //SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL
  #ifdef __midl
    [range(0, 64)]
  #endif //#ifdef __midl
    DWORD                       cDataItems;                 //Number of data items in pDataItems array
  #ifdef __midl
    [size_is(cDataItems)]
  #endif //#ifdef __midl
    PSERVICE_TRIGGER_SPECIFIC_DATA_ITEM  pDataItems;        //Trigger specific data
  } SERVICE_TRIGGER, *PSERVICE_TRIGGER;
}

__if_not_exists (SERVICE_TRIGGER_INFO)
{
  typedef struct _SERVICE_TRIGGER_INFO 
  {
  #ifdef __midl
    [range(0, 64)]
  #endif //#ifdef __midl
    DWORD                   cTriggers;  //Number of triggers in the pTriggers array
  #ifdef __midl
    [size_is(cTriggers)]
  #endif //#ifdef __midl
    PSERVICE_TRIGGER        pTriggers;  //Array of triggers
    PBYTE                   pReserved;  //Reserved, must be NULL
  } SERVICE_TRIGGER_INFO, *PSERVICE_TRIGGER_INFO;
}

__if_not_exists (SERVICE_FAILURE_ACTIONS_FLAG)
{
  typedef struct _SERVICE_FAILURE_ACTIONS_FLAG 
  {
    BOOL fFailureActionsOnNonCrashFailures; //Failure actions flag
  } SERVICE_FAILURE_ACTIONS_FLAG, *LPSERVICE_FAILURE_ACTIONS_FLAG;
}

__if_not_exists (SERVICE_DELAYED_AUTO_START_INFO)
{
  typedef struct _SERVICE_DELAYED_AUTO_START_INFO 
  {
    BOOL fDelayedAutostart; //Delayed autostart flag
  } SERVICE_DELAYED_AUTO_START_INFO, *LPSERVICE_DELAYED_AUTO_START_INFO;
}

__if_not_exists (SERVICE_TIMECHANGE_INFO)
{
  typedef struct _SERVICE_TIMECHANGE_INFO 
  {
    LARGE_INTEGER liNewTime; //New time
    LARGE_INTEGER liOldTime; //Old time
  } SERVICE_TIMECHANGE_INFO, *PSERVICE_TIMECHANGE_INFO;
}

__if_not_exists (WTSSESSION_NOTIFICATION)
{
  typedef struct tagWTSSESSION_NOTIFICATION
  {
    DWORD cbSize;
    DWORD dwSessionId;
  } WTSSESSION_NOTIFICATION, *PWTSSESSION_NOTIFICATION;
}

#ifndef SERVICE_CONFIG_DELAYED_AUTO_START_INFO
#define SERVICE_CONFIG_DELAYED_AUTO_START_INFO 3
#endif //#ifndef SERVICE_CONFIG_DELAYED_AUTO_START_INFO

#ifndef SERVICE_CONFIG_FAILURE_ACTIONS_FLAG
#define SERVICE_CONFIG_FAILURE_ACTIONS_FLAG 4
#endif //#ifndef SERVICE_CONFIG_FAILURE_ACTIONS_FLAG

#ifndef SERVICE_CONFIG_SERVICE_SID_INFO
#define SERVICE_CONFIG_SERVICE_SID_INFO 5
#endif //#ifndef SERVICE_CONFIG_SERVICE_SID_INFO

#ifndef SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO
#define SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO 6
#endif //#ifndef SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO

#ifndef SERVICE_CONFIG_PRESHUTDOWN_INFO
#define SERVICE_CONFIG_PRESHUTDOWN_INFO 7
#endif //#ifndef SERVICE_CONFIG_PRESHUTDOWN_INFO

#ifndef SERVICE_CONFIG_TRIGGER_INFO
#define SERVICE_CONFIG_TRIGGER_INFO 8
#endif //#ifndef SERVICE_CONFIG_TRIGGER_INFO

#ifndef SERVICE_CONFIG_PREFERRED_NODE
#define SERVICE_CONFIG_PREFERRED_NODE 9
#endif //#ifndef SERVICE_CONFIG_PREFERRED_NODE

#ifndef SERVICE_CONFIG_LAUNCH_PROTECTED
#define SERVICE_CONFIG_LAUNCH_PROTECTED 12
#endif //#ifndef SERVICE_CONFIG_LAUNCH_PROTECTED

#ifndef SERVICE_LAUNCH_PROTECTED_NONE
#define SERVICE_LAUNCH_PROTECTED_NONE 0
#endif //#ifndef SERVICE_LAUNCH_PROTECTED_NONE

__if_not_exists (SERVICE_SID_INFO)
{
  typedef struct _SERVICE_SID_INFO 
  {
    DWORD dwServiceSidType; //Service SID type
  } SERVICE_SID_INFO, *LPSERVICE_SID_INFO;
}

__if_not_exists (SERVICE_REQUIRED_PRIVILEGES_INFOA)
{
  typedef struct _SERVICE_REQUIRED_PRIVILEGES_INFOA 
  {
    LPSTR pmszRequiredPrivileges; //Required privileges multi-sz
  } SERVICE_REQUIRED_PRIVILEGES_INFOA, *LPSERVICE_REQUIRED_PRIVILEGES_INFOA;
}

__if_not_exists (SERVICE_REQUIRED_PRIVILEGES_INFOW)
{
  typedef struct _SERVICE_REQUIRED_PRIVILEGES_INFOW 
  {
    LPWSTR pmszRequiredPrivileges; //Required privileges multi-sz
  } SERVICE_REQUIRED_PRIVILEGES_INFOW, *LPSERVICE_REQUIRED_PRIVILEGES_INFOW;
}

#ifdef _UNICODE
__if_not_exists (SERVICE_REQUIRED_PRIVILEGES_INFO)
{
  typedef SERVICE_REQUIRED_PRIVILEGES_INFOW SERVICE_REQUIRED_PRIVILEGES_INFO;
}

__if_not_exists (LPSERVICE_REQUIRED_PRIVILEGES_INFO)
{
  typedef LPSERVICE_REQUIRED_PRIVILEGES_INFOW LPSERVICE_REQUIRED_PRIVILEGES_INFO;
}
#else
__if_not_exists (SERVICE_REQUIRED_PRIVILEGES_INFO)
{
  typedef SERVICE_REQUIRED_PRIVILEGES_INFOA SERVICE_REQUIRED_PRIVILEGES_INFO;
}

__if_not_exists (LPSERVICE_REQUIRED_PRIVILEGES_INFO)
{
  typedef LPSERVICE_REQUIRED_PRIVILEGES_INFOA LPSERVICE_REQUIRED_PRIVILEGES_INFO;
}
#endif //#ifdef _UNICODE

__if_not_exists (SERVICE_PRESHUTDOWN_INFO)
{
  typedef struct _SERVICE_PRESHUTDOWN_INFO 
  {
    DWORD dwPreshutdownTimeout; //Timeout in msecs
  } SERVICE_PRESHUTDOWN_INFO, *LPSERVICE_PRESHUTDOWN_INFO;
}

__if_not_exists (SERVICE_PREFERRED_NODE_INFO)
{
  typedef struct _SERVICE_PREFERRED_NODE_INFO 
  {
    USHORT  usPreferredNode; //Preferred node
    BOOLEAN fDelete;         //Delete the preferred node setting
  } SERVICE_PREFERRED_NODE_INFO, *LPSERVICE_PREFERRED_NODE_INFO;
}

__if_not_exists (_SERVICE_LAUNCH_PROTECTED_INFO)
{
  typedef struct _SERVICE_LAUNCH_PROTECTED_INFO 
  {
    DWORD dwLaunchProtected; //Service launch protected
  } SERVICE_LAUNCH_PROTECTED_INFO, *PSERVICE_LAUNCH_PROTECTED_INFO;
}

__if_not_exists (SERVICE_CONTROL_STATUS_REASON_PARAMSA)
{
  typedef struct _SERVICE_CONTROL_STATUS_REASON_PARAMSA 
  {
    DWORD                   dwReason;
    LPSTR                   pszComment;
    SERVICE_STATUS_PROCESS  ServiceStatus;
  } SERVICE_CONTROL_STATUS_REASON_PARAMSA, *PSERVICE_CONTROL_STATUS_REASON_PARAMSA;
}

__if_not_exists (SERVICE_CONTROL_STATUS_REASON_PARAMSW)
{
  typedef struct _SERVICE_CONTROL_STATUS_REASON_PARAMSW 
  {
    DWORD                  dwReason;
    LPWSTR                 pszComment;
    SERVICE_STATUS_PROCESS ServiceStatus;
  } SERVICE_CONTROL_STATUS_REASON_PARAMSW, *PSERVICE_CONTROL_STATUS_REASON_PARAMSW;
}  
  
#ifdef _UNICODE
__if_not_exists (SERVICE_CONTROL_STATUS_REASON_PARAMS)
{
  typedef SERVICE_CONTROL_STATUS_REASON_PARAMSW SERVICE_CONTROL_STATUS_REASON_PARAMS;
}

__if_not_exists (PSERVICE_CONTROL_STATUS_REASON_PARAMS)
{
  typedef PSERVICE_CONTROL_STATUS_REASON_PARAMSW PSERVICE_CONTROL_STATUS_REASON_PARAMS;
}
#else
__if_not_exists (SERVICE_CONTROL_STATUS_REASON_PARAMS)
{
  typedef SERVICE_CONTROL_STATUS_REASON_PARAMSA SERVICE_CONTROL_STATUS_REASON_PARAMS;
}

__if_not_exists (PSERVICE_CONTROL_STATUS_REASON_PARAMS)
{
  typedef PSERVICE_CONTROL_STATUS_REASON_PARAMSA PSERVICE_CONTROL_STATUS_REASON_PARAMS;
}
#endif //#ifdef _UNICODE

#ifndef SERVICE_CONTROL_PRESHUTDOWN
#define SERVICE_CONTROL_PRESHUTDOWN 0x0000000F
#endif //#ifndef SERVICE_CONTROL_PRESHUTDOWN

#ifndef SERVICE_CONTROL_TIMECHANGE
#define SERVICE_CONTROL_TIMECHANGE 0x00000010
#endif //#ifndef SERVICE_CONTROL_TIMECHANGE

#ifndef SERVICE_CONTROL_TRIGGEREVENT
#define SERVICE_CONTROL_TRIGGEREVENT 0x00000020
#endif //#ifndef SERVICE_CONTROL_TRIGGEREVENT

#ifndef SERVICE_SID_TYPE_NONE
#define SERVICE_SID_TYPE_NONE 0x00000000
#endif //#ifndef SERVICE_SID_TYPE_NONE

#ifndef SERVICE_CONTROL_STATUS_REASON_INFO
#define SERVICE_CONTROL_STATUS_REASON_INFO 1
#endif //#ifndef SERVICE_CONTROL_STATUS_REASON_INFO

#ifndef _Return_type_success_
#define _Return_type_success_(expr)
#endif //#ifndef _Return_type_success_

#ifndef _In_reads_bytes_opt_
#define _In_reads_bytes_opt_(size)
#endif //#ifndef _In_reads_opt_

#ifndef _In_reads_opt_
#define _In_reads_opt_(size)
#endif //#ifndef _In_reads_opt_

#ifndef _Must_inspect_result_
#define _Must_inspect_result_
#endif //#ifndef _Must_inspect_result_

#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(size)
#endif 

#ifndef _Outptr_
#define _Outptr_
#endif //#ifndef _Outptr_

#ifndef _In_opt_
#define _In_opt_
#endif //#ifndef _In_opt_

#ifndef _In_
#define _In_
#endif //#ifndef _In_

#ifndef _Out_opt_
#define _Out_opt_
#endif //#ifndef _Out_opt_

#ifndef _Inout_
#define _Inout_
#endif //#ifndef _Inout_

#ifndef _Out_
#define _Out_
#endif //#ifndef _Out_

#ifndef _In_opt_z_
#define _In_opt_z_
#endif //#ifndef _In_opt_z_

#ifndef _In_z_
#define _In_z_
#endif //#ifndef _In_z_

#ifndef _Inout_opt_
#define _Inout_opt_
#endif //#ifndef _Inout_opt_


#endif //#ifndef __NTSERVDEFINES_H__
