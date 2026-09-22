#include <windows.h>
#include <tlhelp32.h>

static void report(const char *message, DWORD value)
{
    char line[256];
    DWORD written;
    wsprintfA(line, "%s: %lu\r\n", message, value);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), line, lstrlenA(line), &written, NULL);
}

static BOOL loaded(DWORD pid)
{
    MODULEENTRY32W entry = {.dwSize = sizeof(entry)};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    BOOL found = FALSE;
    if (snapshot == INVALID_HANDLE_VALUE) return FALSE;
    if (Module32FirstW(snapshot, &entry)) do {
        if (!lstrcmpiW(entry.szModule, L"inventor-raytracing-startup.dll")) {
            found = TRUE;
            break;
        }
    } while (Module32NextW(snapshot, &entry));
    CloseHandle(snapshot);
    return found;
}

static DWORD inject(DWORD pid)
{
    const WCHAR path[] = L"C:\\ResearchUI\\inventor-raytracing-startup.dll";
    HANDLE process, thread;
    void *remote;
    DWORD wait, code = 0, result = 1;
    if (loaded(pid)) {
        report("Compatibility DLL already loaded in PID", pid);
        return 0;
    }
    process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    if (!process) { report("OpenProcess failed", GetLastError()); return 1; }
    remote = VirtualAllocEx(process, NULL, sizeof(path), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) { report("VirtualAllocEx failed", GetLastError()); goto done; }
    if (!WriteProcessMemory(process, remote, path, sizeof(path), NULL)) {
        report("WriteProcessMemory failed", GetLastError()); goto free_remote;
    }
    thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, remote, 0, NULL);
    if (!thread) { report("CreateRemoteThread failed", GetLastError()); goto free_remote; }
    wait = WaitForSingleObject(thread, 30000);
    if (wait != WAIT_OBJECT_0) {
        report("LoadLibrary wait did not complete", wait);
        CloseHandle(thread);
        /* The remote thread may still read the path. Do not free it or kill it. */
        goto done;
    }
    if (!GetExitCodeThread(thread, &code)) report("GetExitCodeThread failed", GetLastError());
    report("LoadLibrary thread result (low 32 bits)", code);
    CloseHandle(thread);
    /* HMODULE is 64-bit: the DWORD thread result alone is not a reliable check. */
    if (loaded(pid)) { report("Verified compatibility DLL in PID", pid); result = 0; }
    else report("Compatibility DLL missing after LoadLibrary in PID", pid);
free_remote:
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
done:
    CloseHandle(process);
    return result;
}

void mainCRTStartup(void)
{
    for (unsigned attempt = 0; attempt < 1200; ++attempt) {
        PROCESSENTRY32W entry = {.dwSize = sizeof(entry)};
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        DWORD pid = 0;
        if (snapshot != INVALID_HANDLE_VALUE) {
            if (Process32FirstW(snapshot, &entry)) do {
                if (!lstrcmpiW(entry.szExeFile, L"Inventor.exe")) { pid = entry.th32ProcessID; break; }
            } while (Process32NextW(snapshot, &entry));
            CloseHandle(snapshot);
        }
        if (pid) ExitProcess(inject(pid));
        Sleep(100);
    }
    report("Timed out waiting for Inventor", ERROR_TIMEOUT);
    ExitProcess(1);
}
