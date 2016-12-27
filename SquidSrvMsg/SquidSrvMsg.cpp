#include "stdafx.h"

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
  if (dwReason == DLL_PROCESS_ATTACH)
    DisableThreadLibraryCalls(hInstance);
  return TRUE;
}

//The following function is provided to ensure that a .lib file
//is generated for TestSrvMsg. This is required as we are using
//the build in Dependency settings in VC to ensure the message
//dll is build before the NT service exe.
void __declspec(dllexport) Dummy()
{
}