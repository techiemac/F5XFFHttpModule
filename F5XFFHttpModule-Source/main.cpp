//===========================================================================
//
// File         : main.cpp
// Description  : RegisterModule DLL entry point for X-Forwarded-For IIS Module.
//                   
//---------------------------------------------------------------------------
//
// The contents of this file are subject to the "END USER LICENSE AGREEMENT FOR F5
// Software Development Kit for iControl"; you may not use this file except in
// compliance with the License. The License is included in the iControl
// Software Development Kit.
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
// the License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is iControl Code and related documentation
// distributed by F5.
//
// The Initial Developer of the Original Code is F5 Networks, Inc.
// Seattle, WA, USA.
// Portions created by F5 are Copyright (C) 2009 F5 Networks, Inc.
// All Rights Reserved.
// iControl (TM) is a registered trademark of F5 Networks, Inc.
//
// Alternatively, the contents of this file may be used under the terms
// of the GNU General Public License (the "GPL"), in which case the
// provisions of GPL are applicable instead of those above.  If you wish
// to allow use of your version of this file only under the terms of the
// GPL and not to allow others to use your version of this file under the
// License, indicate your decision by deleting the provisions above and
// replace them with the notice and other provisions required by the GPL.
// If you do not delete the provisions above, a recipient may use your
// version of this file under either the License or the GPL.
//
//===========================================================================
#include "precomp.h"


IHttpServer *g_pHttpServer = NULL;  //  Global server instance
PVOID g_pModuleContext = NULL;  //  Global module context id
CRITICAL_SECTION g_CS;
static const DWORD BUFFER_LEN = 256;
static TCHAR *DEFAULT_HEADER_NAME = _T("X-Forwarded-For");
TCHAR gHEADER_NAME[BUFFER_LEN];

//---------------------------------------------------------------------------
//  The RegisterModule entrypoint implementation.
//  This method is called by the server when the module DLL is 
//  loaded in order to create the module factory,
//  and register for server events.
//---------------------------------------------------------------------------
HRESULT __stdcall
RegisterModule(DWORD dwServerVersion, IHttpModuleRegistrationInfo *pModuleInfo, IHttpServer *pHttpServer)
{
    HRESULT hr = S_OK;
    CF5XFFHttpModuleFactory *pFactory = NULL;

    if ( pModuleInfo == NULL || pHttpServer == NULL )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
    }
	else
	{
		// step 1: save the IHttpServer and the module context id for future use
		g_pModuleContext = pModuleInfo->GetId();
		g_pHttpServer = pHttpServer;

		// step 2: create the module factory
		pFactory = new CF5XFFHttpModuleFactory(gHEADER_NAME);
		if ( pFactory == NULL )
		{
			hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
		}
		else
		{
			// step 3: register for server events
			// TODO: register for more server events here
			DWORD dwRequestNotifications = RQ_BEGIN_REQUEST | RQ_SEND_RESPONSE;
			DWORD dwPostRequestNotifications = 0;

			hr = pModuleInfo->SetRequestNotifications( pFactory, dwRequestNotifications, dwPostRequestNotifications);
			if ( SUCCEEDED(hr) )
			{
				// Set priority so that we can get some log data info in the SendResponse event.
				hr = pModuleInfo->SetPriorityForRequestNotification(RQ_SEND_RESPONSE,PRIORITY_ALIAS_HIGH);
				if ( SUCCEEDED(hr) )
				{
					// Everything is good so set pFactory to NULL so it's not deleted below.
					pFactory = NULL;
				}
			}
		}
	}

    if ( pFactory != NULL )
    {
        delete pFactory;
        pFactory = NULL;
    }   

    return hr;
}

void 
DebugMsg(TCHAR *szFormat, ...)
{
#ifdef _DEBUG
	TCHAR szBuf[1024];
	va_list list;
	va_start(list, szFormat);
	vsprintf(szBuf, szFormat, list);

	HANDLE hFile = CreateFile(_T("c:\\F5XFFModule.log"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE != hFile )
	{
		DWORD dwWritten;
		SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
		WriteFile(hFile, _T("\r\n"), 2, &dwWritten, NULL);
		CloseHandle(hFile);
	}

	va_end(list);
#endif
}

void
ReadConfiguration( HANDLE hModule )
{
	TCHAR configPath[BUFFER_LEN];
	DWORD dwLen = 0;

	memset(gHEADER_NAME, '\0', BUFFER_LEN * sizeof(TCHAR));
	memset(configPath, '\0', BUFFER_LEN * sizeof(TCHAR));

	DebugMsg("============================================");
	dwLen = GetModuleFileName((HMODULE)hModule, configPath, 256);
	if (dwLen > 4)
	{
		// convert .dll to .ini
		configPath[dwLen-3] = 'i';
		configPath[dwLen-2] = 'n';
		configPath[dwLen-1] = 'i';

		DebugMsg(_T("Reading profile information from '%s'"), configPath);

		DWORD dwStat = GetPrivateProfileString(
			_T("SETTINGS"), _T("HEADER"), DEFAULT_HEADER_NAME,
			gHEADER_NAME, BUFFER_LEN,
			configPath);
		DebugMsg(_T("Using custom header value of '%s'"), gHEADER_NAME);
	}
	if ( _T('\0') == gHEADER_NAME[0] )
	{
		_tcscpy(gHEADER_NAME, DEFAULT_HEADER_NAME);
	}
	//if ( _T(':') != gHEADER_NAME[_tcslen(gHEADER_NAME)-1] )
	//{
		//_tcscat(gHEADER_NAME, _T(":"));
	//}
}


BOOL APIENTRY
DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{

			InitializeCriticalSection(&g_CS);
			ReadConfiguration(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DeleteCriticalSection(&g_CS);
			break;
		}
	}
	return TRUE;
}