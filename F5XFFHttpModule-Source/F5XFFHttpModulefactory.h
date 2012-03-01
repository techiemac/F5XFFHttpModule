//===========================================================================
//
// File         : F5XFFHttpModuleFactory.h
// Description  : Factory class used for creating F5XFFHttpModule classes.
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
#ifndef __MODULE_FACTORY_H__
#define __MODULE_FACTORY_H__

#include <stdio.h>

//---------------------------------------------------------------------------
// class CF5XFFHttpModuleFactory
//
// Factory class for CF5XFFHttpModule.
// This class is responsible for creating instances
// of CF5XFFHttpModule for each request.
//---------------------------------------------------------------------------
class CF5XFFHttpModuleFactory : public IHttpModuleFactory
{
	public:
		TCHAR m_HeaderName[1024];

	public:
		CF5XFFHttpModuleFactory()
		{
			SetHeaderName(_T("X-Forwarded-For"));
		}

		CF5XFFHttpModuleFactory(TCHAR *szHeaderName)
		{
			SetHeaderName(szHeaderName);
		}

	public:
		void SetHeaderName(TCHAR *szHeaderName)
		{
			if ( NULL != szHeaderName )
			{
				_tcsncpy(m_HeaderName, szHeaderName, 1023);
			}
		}

	public:
		virtual HRESULT
		GetHttpModule( OUT CHttpModule **ppModule, IN IModuleAllocator *)
		{
			HRESULT hr = S_OK;
			CF5XFFHttpModule *pModule = NULL;

			if ( ppModule == NULL )
			{
				hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
				goto Finished;
			}

			pModule = new CF5XFFHttpModule(m_HeaderName);
			if ( pModule == NULL )
			{
				hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
				goto Finished;
			}

			*ppModule = pModule;
			pModule = NULL;
	            
		Finished:

			if ( pModule != NULL )
			{
				delete pModule;
				pModule = NULL;
			}

			return hr;
		}

		virtual void
		Terminate()
		{
			delete this;
		}
};

#endif
