/*
Module : ntservScmService.h
Purpose: Defines the interface for the class CNTScmService.
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

#ifndef __NTSERVSCMSERVICE_H__
#define __NTSERVSCMSERVICE_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//Forward declaration
class CNTServiceControlManager;

//An encapsulation of a service as returned from querying the SCM (i.e. an SC_HANDLE)
class CNTSERVICE_EXT_CLASS CNTScmService
{
public:
//typedefs
  typedef BOOL (CALLBACK* ENUM_SERVICES_PROC)(void* pUserData, ENUM_SERVICE_STATUS& ss);

//Constructors / Destructors
  CNTScmService();
  ~CNTScmService();

//Methods
  //Releases the underlying SC_HANDLE
  void Close();

  //Allows access to the underlying SC_HANDLE representing the service
  operator SC_HANDLE() const;

  //Attach / Detach support from an SDK SC_HANDLE
  _Return_type_success_(return != 0) BOOL Attach(_In_opt_ SC_HANDLE hService);
  SC_HANDLE Detach();

  //Changes the configuration of this service
  _Return_type_success_(return != 0) BOOL ChangeConfig(_In_ DWORD dwServiceType, _In_ DWORD dwStartType, _In_ DWORD dwErrorControl,
                                                       _In_opt_ LPCTSTR lpBinaryPathName, _In_opt_ LPCTSTR lpLoadOrderGroup, _Out_opt_ LPDWORD lpdwTagId,
                                                       _In_opt_ LPCTSTR lpDependencies, _In_opt_ LPCTSTR lpServiceStartName, _In_opt_ LPCTSTR lpPassword,
                                                       _In_opt_ LPCTSTR lpDisplayName) const;

  //Send a command to the service
  _Return_type_success_(return != 0) BOOL Control(_In_ DWORD dwControl);
  _Return_type_success_(return != 0) BOOL Control(_In_ DWORD dwControl, _In_ DWORD dwInfoLevel, _Inout_ PVOID pControlParams);

  //These functions call Control() with the standard predefined control codes
  _Return_type_success_(return != 0) BOOL Stop() const;			  //Ask the service to stop
  _Return_type_success_(return != 0) BOOL Pause() const;			  //Ask the service to pause
  _Return_type_success_(return != 0) BOOL Continue() const;	  //Ask the service to continue
  _Return_type_success_(return != 0) BOOL Interrogate() const; //Ask the service to update its status to the SCM

  //Waits for a service to stop with a configurable timeout
  _Return_type_success_(return != 0) BOOL WaitForStop(_In_ DWORD dwTimeout);

  //Waits for a service to have a certain status (with a configurable timeout)
  _Return_type_success_(return != 0) BOOL WaitForServiceStatus(_In_ DWORD dwTimeout, _In_ DWORD dwWaitForStatus, _In_ DWORD dwPollingInterval = 250);

  //Start the execution of the service
  _Return_type_success_(return != 0) BOOL Start(_In_ DWORD dwNumServiceArgs, _In_reads_opt_(dwNumServiceArgs) LPCTSTR* lpServiceArgVectors) const;

  //Determines what Control codes this service supports
  _Return_type_success_(return != 0) BOOL AcceptStop(_Out_ BOOL& bStop);                   //Ask the service can it stop
  _Return_type_success_(return != 0) BOOL AcceptPauseContinue(_Out_ BOOL& bPauseContinue);	//Ask the service can it pause continue
  _Return_type_success_(return != 0) BOOL AcceptShutdown(_Out_ BOOL& bShutdown);           //Ask the service if it is notified of shutdowns

  //Get the most return status of the service reported to the SCM by this service
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryStatus(_Out_ SERVICE_STATUS& ServiceStatus) const;

  //Get the configuration parameters of this service from the SCM
  _Return_type_success_(return != 0) BOOL QueryConfig(_Inout_ LPQUERY_SERVICE_CONFIG& lpServiceConfig) const;

  //Add a new service to the SCM database
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL Create(_In_ CNTServiceControlManager& Manager, _In_ LPCTSTR lpServiceName, _In_opt_ LPCTSTR lpDisplayName,
                                                                       _In_ DWORD dwDesiredAccess, _In_ DWORD dwServiceType, _In_ DWORD dwStartType, _In_ DWORD dwErrorControl,	             
                                                                       _In_opt_ LPCTSTR lpBinaryPathName, _In_opt_ LPCTSTR lpLoadOrderGroup, _Out_opt_ LPDWORD lpdwTagId, _In_opt_ LPCTSTR lpDependencies,
                                                                       _In_opt_ LPCTSTR lpServiceStartName, _In_opt_ LPCTSTR lpPassword);

  //Mark this service as to be deleted from the SCM.
  _Return_type_success_(return != 0) BOOL Delete() const;

  //Enumerate the services that this service depends upon
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL EnumDependents(_In_ DWORD dwServiceState, _In_opt_ void* pUserData, _In_ ENUM_SERVICES_PROC lpEnumServicesFunc) const;

  //Get the security information associated with this service
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryObjectSecurity(_In_ SECURITY_INFORMATION dwSecurityInformation, _Inout_ PSECURITY_DESCRIPTOR& lpSecurityDescriptor) const;

  //Set the security descriptor associated with this service
  _Return_type_success_(return != 0) BOOL SetObjectSecurity(_In_ SECURITY_INFORMATION dwSecurityInformation, _In_ PSECURITY_DESCRIPTOR lpSecurityDescriptor) const;

  //Windows 2000+ specific functions
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryDescription(_Out_ CNTServiceString& sDescription) const;
  _Return_type_success_(return != 0) BOOL ChangeDescription(_In_ const CNTServiceString& sDescription);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryFailureActions(_Inout_ LPSERVICE_FAILURE_ACTIONS& pActions) const;
  _Return_type_success_(return != 0) BOOL ChangeFailureActions(_In_opt_ LPSERVICE_FAILURE_ACTIONS pActions);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryStatus(_Out_ SERVICE_STATUS_PROCESS& ssp) const;

  //Windows Vista / 2008+ specific functions
  DWORD NotifyStatusChange(_In_ DWORD dwNotifyMask, _In_ PSERVICE_NOTIFY pNotifyBuffer) const;
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL  QueryDelayAutoStart(_Out_ BOOL& bDelayedAutoStart) const;
  _Return_type_success_(return != 0) BOOL  ChangeDelayAutoStart(_In_ BOOL bDelayedAutoStart);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL  QueryFailureActionsFlag(_Out_ BOOL& bFailureActionsOnNonCrashFailures) const;
  _Return_type_success_(return != 0) BOOL  ChangeFailureActionsFlag(_In_ BOOL bFailureActionsOnNonCrashFailures);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL  QuerySidInfo(_Out_ DWORD& dwServiceSidType) const;
  _Return_type_success_(return != 0) BOOL  ChangeSidInfo(_In_ DWORD dwServiceSidType);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL  QueryRequiredPrivileges(_Out_ CNTServiceStringArray& privileges) const;
  _Return_type_success_(return != 0) BOOL  ChangeRequiredPrivileges(_In_ const CNTServiceStringArray& priviledges);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL  QueryPreShutdown(_Out_ DWORD& dwPreshutdownTimeout) const;
  _Return_type_success_(return != 0) BOOL  ChangePreShutdown(_In_ DWORD dwPreshutdownTimeout);

  //Windows 7 / 2008 RC+ specific functions
  _Return_type_success_(return != 0) BOOL ChangeTrigger(_In_ PSERVICE_TRIGGER_INFO pTriggerInfo);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryTrigger(_Inout_ PSERVICE_TRIGGER_INFO& pTriggerInfo) const;
  _Return_type_success_(return != 0) BOOL ChangePreferredNode(_In_ USHORT usPreferredNode, _In_ BOOL bDelete);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryPreferredNode(_Out_ USHORT& usPreferredNode, _Out_ BOOL& bDelete) const;

  //Windows 8.1+ specific functions
  _Return_type_success_(return != 0) BOOL ChangeLaunchProtected(_In_ DWORD dwLaunchProtected);
  _Must_inspect_result_ _Return_type_success_(return != 0) BOOL QueryLaunchProtected(_Out_ DWORD& dwLaunchProtected) const;

protected:
//typedefs
  typedef BOOL (WINAPI QUERYSERVICECONFIG2)(SC_HANDLE, DWORD, LPBYTE, DWORD, LPDWORD);
  typedef QUERYSERVICECONFIG2* LPQUERYSERVICECONFIG2;
  typedef BOOL (WINAPI CHANGESERVICECONFIG2)(SC_HANDLE, DWORD, LPVOID);
  typedef CHANGESERVICECONFIG2* LPCHANGESERVICECONFIG2;
  typedef BOOL (WINAPI QUERYSERVICESTATUSEX)(SC_HANDLE, SC_STATUS_TYPE, LPBYTE, DWORD, LPDWORD);
  typedef QUERYSERVICESTATUSEX* LPQUERYSERVICESTATUSEX;
  typedef BOOL (WINAPI NOTIFYSERVICESTATUSCHANGE)(SC_HANDLE, DWORD, PSERVICE_NOTIFY);
  typedef NOTIFYSERVICESTATUSCHANGE* LPNOTIFYSERVICESTATUSCHANGE;
  typedef BOOL (WINAPI CONTROLSERVICEEX)(SC_HANDLE, DWORD, DWORD, PVOID);
  typedef CONTROLSERVICEEX* LPCONTROLSERVICEEX;

//Member variables
  SC_HANDLE                   m_hService;
  HINSTANCE                   m_hAdvapi32;                     //Instance handle of the "ADVAPI32.DLL" which houses all of the NT Service functions
  LPQUERYSERVICECONFIG2       m_lpfnQueryServiceConfig2;       //QueryServiceConfig2 function pointer
  LPCHANGESERVICECONFIG2      m_lpfnChangeServiceConfig2;      //ChangeServiceConfig2 function pointer
  LPQUERYSERVICESTATUSEX      m_lpfnQueryServiceStatusEx;      //QueryServiceStatusEx function pointer
  LPNOTIFYSERVICESTATUSCHANGE m_lpfnNotifyServiceStatusChange; //NotifyServiceStatusChange function pointer
  LPCONTROLSERVICEEX          m_lpfnControlServiceEx;          //ControlServiceEx function pointer
};

#endif //#ifndef __NTSERVSCMSERVICE_H__
