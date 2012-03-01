//===========================================================================
//
// File         : F5XFFHttpModule.h
// Description  : Native HTTP Module replacing c-ip log value with 
//                X-Forwarding-For HTTP header
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
#ifndef __MY_MODULE_H__
#define __MY_MODULE_H__

#include <stdio.h>

//---------------------------------------------------------------------------
// class CF5XFFHttpModule 
//
// This class implements the CHTTPModule methods needed to replace the c-ip
// log value with a supplied X-Forwarded-For header.
//---------------------------------------------------------------------------
class CF5XFFHttpModule : public CHttpModule
{

	//-----------------------------------------------------------------------
	// Members
	//-----------------------------------------------------------------------
	public:
		TCHAR *m_pszHeaderValue;
		TCHAR m_pszHeaderName[1024];
		HANDLE m_hEventLog;

	//-----------------------------------------------------------------------
	// Constructors
	//-----------------------------------------------------------------------
	public:
		CF5XFFHttpModule()
		{
			Initialize(_T("X-Forwarded-For"));
		}

		CF5XFFHttpModule(TCHAR *szHeaderName)
		{
			Initialize(szHeaderName);
		}

		~CF5XFFHttpModule()
		{
			if ( NULL != m_hEventLog )
			{
				DebugMessage(_T("<-- Deregistering F5 X-Forwarded-For Http Module..."));
				DeregisterEventSource(m_hEventLog);
				m_hEventLog = NULL;
			}
		}

	//-----------------------------------------------------------------------
	// Member Accessors
	//-----------------------------------------------------------------------
	public:
		void Initialize(TCHAR *szHeaderName)
		{
			m_pszHeaderValue = NULL;
			SetHeaderName(szHeaderName);

			m_hEventLog = RegisterEventSource(NULL, _T("IISADMIN"));

			DebugMessage(_T("--> Registering F5 X-Forwarded-For Http Module..."));
		}

		TCHAR *GetHeaderName()
		{
			return m_pszHeaderName;
		}

		void SetHeaderName(TCHAR *szHeaderName)
		{
			if ( NULL != szHeaderName )
			{
				_tcsncpy(m_pszHeaderName, szHeaderName, 1023);
			}
		}

	//-----------------------------------------------------------------------
	// Internal Methods
	//-----------------------------------------------------------------------
	public:
		void DebugMessage(TCHAR *szFormat, ...);
		bool WriteFileLogMessage(TCHAR *szMsg);
		bool WriteEventLogMessage(TCHAR *szMsg);

	//-----------------------------------------------------------------------
	// CHttpModule methods
	//-----------------------------------------------------------------------
	public:
		REQUEST_NOTIFICATION_STATUS
		OnBeginRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider);

		REQUEST_NOTIFICATION_STATUS
		OnSendResponse(IN IHttpContext * pHttpContext, IN ISendResponseProvider * pProvider);

};

#endif
