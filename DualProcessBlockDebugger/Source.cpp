#include <Windows.h>
#include <cstdio>

#define IS_PARRENT_PROCESS 0
#define IS_SUB_PROCESS 1
const char mutexname[] = "re::m";

DWORD pid = 0;
HANDLE mutex;

int doParrentProcess();
int doChildProcess();

int func() {
	printf("[%d] creating mutex\n", pid);
	mutex = CreateMutexA(NULL, FALSE, mutexname);
	if (!mutex) {
		DWORD error = GetLastError();
		printf("CreateMutex failed [%d]\n", error);
		return -1;
		// is subprocess
	}
	DWORD error = GetLastError();
	printf("[%d] mutex: %p, %d\n", pid, mutex, error);
	return (ERROR_ALREADY_EXISTS == error) ? IS_SUB_PROCESS : IS_PARRENT_PROCESS;
}


int main() {
	pid = GetCurrentProcessId();
	printf("[%d] Started...\n", pid);
	switch (func()) {
	case -1:
		exit(0);
		break;
	case IS_PARRENT_PROCESS:
		doParrentProcess();
		break;
	case IS_SUB_PROCESS:
		doChildProcess();
		break;
	default:
		exit(0);
	}
	if (mutex) {
		ReleaseMutex(mutex);
	}
	return 0;
}

int doParrentProcess()
{
	printf("[*] Parrent desu\n");
	char nameBuf[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, nameBuf, MAX_PATH);
	printf("[*] Current: %s\n", nameBuf);

	STARTUPINFOA si = { 0, };
	PROCESS_INFORMATION pi = { 0, };
	if (!CreateProcessA(NULL, nameBuf,
		NULL, NULL, FALSE,
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
		NULL, NULL,
		&si, &pi)) {
		printf("[*] createprocess failed, %d\n", GetLastError());
		return -1;
	}

	printf("[*] childprocess created\n");


	DEBUG_EVENT de = { 0, };
	PVOID excpAddr;
	DWORD excpCode;

	BOOL myDbgTriggerHandled = FALSE;
	BOOL killed = FALSE;
	CONTEXT ctx = { 0, };
	// Debugging
	while (TRUE) {
		ZeroMemory(&de, sizeof(DEBUG_EVENT));

		if (!WaitForDebugEvent(&de, INFINITE)) {
			printf("[*] WaitForDebugEvent failed: %d\n", GetLastError());
			break;
		}

		switch (de.dwDebugEventCode) {
		case EXCEPTION_DEBUG_EVENT:
			excpAddr = de.u.Exception.ExceptionRecord.ExceptionAddress;
			excpCode = de.u.Exception.ExceptionRecord.ExceptionCode;
			//printf("[*] excp code %x\n", excpCode);

			if (excpCode != EXCEPTION_ACCESS_VIOLATION) {
				break;
			} else if (!myDbgTriggerHandled) {
				ctx.ContextFlags = CONTEXT_FULL;
				GetThreadContext(pi.hThread, &ctx);
				ctx.Rip += 7;
				SetThreadContext(pi.hThread, &ctx);
				break;
			}

			printf("[*] unhandled exception: %x\n", excpCode);
			break;
		case EXIT_PROCESS_DEBUG_EVENT:
			printf("Killed");
			killed = TRUE;
			break;
		}

		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
		//printf("[*] continue de\n");
		if (killed) {
			break;
		}
	}

	if (pi.hThread) {
		CloseHandle(pi.hThread);
	}

	return 0;
}

int doChildProcess()
{
	int a = *(int*)NULL;
	DWORD res = MessageBoxA(NULL, "Child proces", "Info", MB_OK);
	printf("[-] ChildProcess done\n");
	return 0;
}