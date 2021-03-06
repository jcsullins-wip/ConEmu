﻿
/*
Copyright (c) 2016 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define HIDE_USE_EXCEPTION_INFO
#define SHOWDEBUGSTR

#include "ConEmuC.h"
#include "../common/MStrDup.h"

#include "Actions.h"
#include "StartEnv.h"

wchar_t* gpszForcedTitle = NULL;

// old-issues#60: On some systems (Win2k3, WinXP) SetConsoleCP and SetConsoleOutputCP just hangs!
// That's why we call them in background thread, and if it hangs - TerminateThread it.
static DWORD WINAPI OurSetConsoleCPThread(LPVOID lpParameter)
{
	UINT nCP = LODWORD(lpParameter);
	SetConsoleCP(nCP);
	SetConsoleOutputCP(nCP);
	return 0;
}

bool SetConsoleCpHelper(UINT nCP)
{
	if (!nCP || nCP > 0xFFFF)
		return false;

	bool bOk = false;

	//Issue 60: BUGBUG: On some OS versions (Win2k3, WinXP) SetConsoleCP (and family) just hangs
	DWORD nTID;
	HANDLE hThread = apiCreateThread(OurSetConsoleCPThread, (LPVOID)(DWORD_PTR)nCP, &nTID, "OurSetConsoleCPThread(%u)", nCP);

	if (hThread)
	{
		DWORD nWait = WaitForSingleObject(hThread, 1000);

		if (nWait == WAIT_TIMEOUT)
		{
			// That is dangerous operation, however there is no other workaround
			// https://conemu.github.io/en/MicrosoftBugs.html#chcp_hung

			apiTerminateThread(hThread, 100);

		}

		CloseHandle(hThread);
	}

	return bOk;
}


CStartEnv::CStartEnv()
{
}

CStartEnv::~CStartEnv()
{
}

void CStartEnv::Alias(LPCWSTR asName, LPCWSTR asValue)
{
	// NULL will remove alias
	// We set aliases for "cmd.exe" executable, as Far Manager supports too
	wchar_t szExe[] = L"cmd.exe";
	// MSDN tells LPCWSTR, but SDK defines the function as LPWSTR
	AddConsoleAlias((LPWSTR)asName, (asValue && *asValue) ? (LPWSTR)asValue : NULL, szExe);
}

void CStartEnv::ChCp(LPCWSTR asCP)
{
	UINT nCP = GetCpFromString(asCP);
	if (nCP > 0 && nCP <= 0xFFFF)
	{
		//Issue 60: BUGBUG: On some OS versions (Win2k3, WinXP) SetConsoleCP (and family) just hangs
		SetConsoleCpHelper(nCP);
	}
}

void CStartEnv::Echo(LPCWSTR asSwitches, LPCWSTR asText)
{
	CEStr lsFull = lstrmerge(asSwitches, (asSwitches && *asSwitches) ? L" " : NULL, L"\"", asText, L"\"");
	DoOutput(ea_OutEcho, lsFull);
}

void CStartEnv::Set(LPCWSTR asName, LPCWSTR asValue)
{
	// Expand value
	wchar_t* pszExpanded = ExpandEnvStr(asValue);
	LPCWSTR pszSet = pszExpanded ? pszExpanded : asValue;
	SetEnvironmentVariable(asName, (pszSet && *pszSet) ? pszSet : NULL);
	SafeFree(pszExpanded);
}

void CStartEnv::Title(LPCWSTR asTitle)
{
	if (asTitle && *asTitle)
	{
		SafeFree(gpszForcedTitle);
		gpszForcedTitle = lstrdup(asTitle);
	}
}

void CStartEnv::Type(LPCWSTR asSwitches, LPCWSTR asFile)
{
	CEStr lsFull = lstrmerge(asSwitches, (asSwitches && *asSwitches) ? L" " : NULL, L"\"", asFile, L"\"");
	DoOutput(ea_OutType, lsFull);
}
