//===========================================================================
//
// File         : F5XFFHttpModule.cpp
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
#include "precomp.h"

//---------------------------------------------------------------------------
// CF5XFFHttpModule::DebugMessage
//---------------------------------------------------------------------------
void 
CF5XFFHttpModule::DebugMessage(TCHAR *szFormat, ...)
{
#ifdef _DEBUG
	TCHAR szBuf[1024];
	va_list list;
	va_start(list, szFormat);
	vsprintf(szBuf, szFormat, list);
	WriteFileLogMessage(szBuf);
	//WriteEventLogMessage(szBuf);
	va_end(list);
#endif
}

//---------------------------------------------------------------------------
// CF5XFFHttpModule::WriteFileLogMessage
//---------------------------------------------------------------------------
bool
CF5XFFHttpModule::WriteFileLogMessage(TCHAR *szMsg)
{
	bool status = FALSE;
	HANDLE hFile = CreateFile(_T("c:\\F5XFFModule.log"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE != hFile )
	{
		DWORD dwWritten;
		SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, szMsg, lstrlen(szMsg), &dwWritten, NULL);
		WriteFile(hFile, _T("\r\n"), 2, &dwWritten, NULL);
		CloseHandle(hFile);
		status = TRUE;
	}
	return status;
}

//---------------------------------------------------------------------------
// CF5XFFHttpModule::WriteEventLogMessage
//---------------------------------------------------------------------------
bool
CF5XFFHttpModule::WriteEventLogMessage(TCHAR *szMsg)
{
	bool status = FALSE;
	if ( (NULL != m_hEventLog) && (NULL != szMsg) )
	{
		status = ReportEvent(m_hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCSTR *)szMsg, NULL);
	}
	return status;
}


//---------------------------------------------------------------------------
//  CF5XFFHttpModule::OnAcquireRequestState
//---------------------------------------------------------------------------
REQUEST_NOTIFICATION_STATUS
CF5XFFHttpModule::OnBeginRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    HRESULT hr = S_OK;

	DebugMessage(_T("(OBR) = OnBeginRequest, header = '%s'"), GetHeaderName());

	IHttpRequest *pRequest = pHttpContext->GetRequest();
	if ( NULL != pRequest )
	{
		TCHAR *pszHeaderValue = NULL;
		USHORT cchHeaderValue = 0;
		pszHeaderValue = (TCHAR *)pRequest->GetHeader(GetHeaderName(), &cchHeaderValue);
		if ( cchHeaderValue > 0 )
		{
			// Allocate space to to store the header
			pszHeaderValue = (TCHAR *)pHttpContext->AllocateRequestMemory(cchHeaderValue+1);
			if ( NULL == pszHeaderValue )
			{
				// Set the error status
				hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				pProvider->SetErrorStatus(hr);
			}
			else
			{
				// Retrieve the header value
				pszHeaderValue = (TCHAR *)pRequest->GetHeader(GetHeaderName(), &cchHeaderValue);
				if ( pszHeaderValue != NULL )
				{
					DebugMessage(_T("(OBR) : Found '%s' with value of '%s'\n"), GetHeaderName(), pszHeaderValue);

					// Check for any additional proxy fields
					// ie. X-Forwarded-For: 10.10.10.10, 1.2.3.4, 5.6.7.8
					TCHAR *sep = _tcschr(pszHeaderValue, _T(','));
					if ( NULL != sep )
					{
						*sep = _T('\0');
						DebugMessage(_T("(OBR) : Proxy detected in value, removing proxy info and using '%s'\n"), pszHeaderValue);
					}
					sep = _tcschr(pszHeaderValue, _T(';'));
					if ( NULL != sep )
					{
						*sep = _T('\0');
						DebugMessage(_T("(OBR) : Proxy detected in value, removing proxy info and using '%s'\n"), pszHeaderValue);
					}

					// point the header value member at the allocated XFF value.
					m_pszHeaderValue = pszHeaderValue;
				}
				else
				{
					DebugMessage(_T("(OBR) : X-Forwarded-For header value not found for URL '%s'"), pRequest->GetRawHttpRequest()->pRawUrl);
				}
			}
		}
	}

    if ( FAILED( hr )  )
    {
        return RQ_NOTIFICATION_FINISH_REQUEST;
    }
    else
    {
        return RQ_NOTIFICATION_CONTINUE;
    }
}

//---------------------------------------------------------------------------
// CF5XFFHttpModule::OnSendResponse
//---------------------------------------------------------------------------
REQUEST_NOTIFICATION_STATUS
CF5XFFHttpModule::OnSendResponse(IN IHttpContext * pHttpContext, IN ISendResponseProvider * pProvider)
{
	HRESULT hr = S_OK;

	DebugMessage(_T("(OSR) = OnSendResponse"));

	if (TRUE == pProvider->GetReadyToLogData())
	{
		if ( NULL != m_pszHeaderValue )
		{
			// Retrieve log information.
			PHTTP_LOG_FIELDS_DATA pLogData = (PHTTP_LOG_FIELDS_DATA)pProvider->GetLogData();
			if ( NULL != pLogData )
			{
				if ( 0 != _tcsncmp(pLogData->ClientIp, m_pszHeaderValue, 128) )
				{
					DebugMessage(_T("(OSR) : Replacing Client IP '%s' with XFF value of '%s'"), pLogData->ClientIp, m_pszHeaderValue);
					pLogData->ClientIp = (PCHAR)m_pszHeaderValue;
					pLogData->ClientIpLength = _tcslen(m_pszHeaderValue);

					hr = pProvider->SetLogData((HTTP_LOG_DATA *)pLogData);
					if ( FAILED(hr) )
					{
						pProvider->SetErrorStatus(hr);
					}
				}
				else
				{
					DebugMessage(_T("(OSR) : Client IP == XFF value == '%s' - Not performing replacement"), m_pszHeaderValue);
				}
			}
			else
			{
				DebugMessage(_T("(OSR) : GetLogData returned NULL"));
			}
		}
		else
		{
			DebugMessage(_T("(OSR) : No XFF header value found"));
		}
	}

    if ( FAILED( hr )  )
    {
        return RQ_NOTIFICATION_FINISH_REQUEST;
    }
    else
    {
        return RQ_NOTIFICATION_CONTINUE;
    }
}

