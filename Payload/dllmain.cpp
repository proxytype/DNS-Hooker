// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include "detours.h"
#include <windns.h>
#include <iostream>

using namespace std;

typedef DNS_STATUS(WINAPI* realDnsQueryEx)(
    PDNS_QUERY_REQUEST pQueryRequest,
    PDNS_QUERY_RESULT  pQueryResults,
    PDNS_QUERY_CANCEL  pCancelHandle
    );

realDnsQueryEx originalRealDnsQueryEx = (realDnsQueryEx)GetProcAddress(GetModuleHandleA("dnsapi.dll"), "DnsQueryEx");

DNS_STATUS WINAPI _DnsQueryEx(
    PDNS_QUERY_REQUEST pQueryRequest,
    PDNS_QUERY_RESULT  pQueryResults,
    PDNS_QUERY_CANCEL  pCancelHandle
) {

    wstring f(pQueryRequest->QueryName);
    size_t found = f.find(L"google");

    if (found != string::npos) {
        return ERROR_INVALID_PARAMETER;
    }

    return originalRealDnsQueryEx(pQueryRequest, pQueryResults, pCancelHandle);

}

void attachDetour() {

    DetourRestoreAfterWith();
    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    DetourAttach((PVOID*)&originalRealDnsQueryEx, _DnsQueryEx);

    DetourTransactionCommit();
}

void deAttachDetour() {

    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    DetourDetach((PVOID*)&originalRealDnsQueryEx, _DnsQueryEx);

    DetourTransactionCommit();
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        attachDetour();
        break;
    case DLL_PROCESS_DETACH:
        deAttachDetour();
        break;
    }
    return TRUE;
}


