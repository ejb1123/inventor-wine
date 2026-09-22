/* Check relative and absolute NT waits against an actual elapsed timer.
 * Run with libfaketime and both settings of WINE_DISABLE_NTSYNC to detect
 * kernel deadlines that incorrectly use the host's unadjusted realtime clock.
 */
#include <windows.h>
#include <stdio.h>

int main(void)
{
    LONG (WINAPI *wait_one)(HANDLE, BOOLEAN, const LARGE_INTEGER *);
    HANDLE event = CreateEventW(NULL, TRUE, FALSE, NULL);
    LARGE_INTEGER timeout, start, end, frequency;
    FILETIME now;
    LONG status;
    double elapsed;
    int failed = 0, absolute;

    setvbuf(stdout, NULL, _IONBF, 0);
    wait_one = (void *)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtWaitForSingleObject");
    if (!event || !wait_one || !QueryPerformanceFrequency(&frequency)) return 2;
    for (absolute = 0; absolute < 2; ++absolute) {
        if (absolute) {
            GetSystemTimeAsFileTime(&now);
            timeout.LowPart = now.dwLowDateTime;
            timeout.HighPart = now.dwHighDateTime;
            timeout.QuadPart += 2000000;
        } else timeout.QuadPart = -2000000;
        QueryPerformanceCounter(&start);
        status = wait_one(event, FALSE, &timeout);
        QueryPerformanceCounter(&end);
        elapsed = 1000.0 * (end.QuadPart - start.QuadPart) / frequency.QuadPart;
        printf("%s: status=0x%08x elapsed_ms=%.2f expected=200\n",
               absolute ? "absolute" : "relative", (unsigned int)status, elapsed);
        if (status != 0x102 || elapsed < 100.0 || elapsed > 5000.0) failed = 1;
    }
    CloseHandle(event);
    return failed;
}
