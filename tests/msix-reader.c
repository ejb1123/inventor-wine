/* Read-only probe for Autodesk's MSIX package reader. No installation or
 * signature bypass: validation option 0 requests full validation.
 * Factory ABI: microsoft/msix-packaging, src/inc/public/AppxPackaging.hpp.
 */
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct appx_factory appx_factory;
typedef struct {
    HRESULT (WINAPI *query_interface)(appx_factory *, REFIID, void **);
    ULONG (WINAPI *add_ref)(appx_factory *);
    ULONG (WINAPI *release)(appx_factory *);
    HRESULT (WINAPI *create_writer)(appx_factory *, IStream *, void *, void **);
    HRESULT (WINAPI *create_reader)(appx_factory *, IStream *, IUnknown **);
} factory_vtable;
struct appx_factory { const factory_vtable *vtbl; };

int main(int argc, char **argv)
{
    WCHAR dll_path[32768], package_path[32768];
    HRESULT (WINAPI *create_factory)(UINT32, appx_factory **);
    HRESULT (WINAPI *create_stream)(LPCWSTR, bool, IStream **);
    appx_factory *factory = NULL;
    IStream *stream = NULL;
    IUnknown *reader = NULL;
    HMODULE module;
    HRESULT hr;
    SYSTEMTIME now;
    ULONGLONG started;
    int result = 1;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc != 3) {
        fprintf(stderr, "usage: msix-reader.exe <Windows msix.dll path> <Windows package path>\n");
        return 2;
    }
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argv[1], -1,
                            dll_path, ARRAYSIZE(dll_path)) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argv[2], -1,
                            package_path, ARRAYSIZE(package_path))) return 2;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) return 2;
    module = LoadLibraryExW(dll_path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!module) {
        fprintf(stderr, "LoadLibraryExW failed: %u\n", (unsigned int)GetLastError());
        CoUninitialize();
        return 2;
    }
    create_factory = (void *)GetProcAddress(module, "CoCreateAppxFactory");
    create_stream = (void *)GetProcAddress(module, "CreateStreamOnFileUTF16");
    if (!create_factory || !create_stream) goto done;
    GetSystemTime(&now);
    printf("Windows UTC: %04u-%02u-%02u %02u:%02u:%02u\n", now.wYear,
           now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
    hr = create_factory(0, &factory);
    printf("CoCreateAppxFactory(full validation): 0x%08x\n", (unsigned int)hr);
    if (FAILED(hr)) goto done;
    hr = create_stream(package_path, true, &stream);
    printf("CreateStreamOnFileUTF16: 0x%08x\n", (unsigned int)hr);
    if (FAILED(hr)) goto done;
    started = GetTickCount64();
    hr = factory->vtbl->create_reader(factory, stream, &reader);
    printf("CreatePackageReader: 0x%08x elapsed_ms=%llu\n",
           (unsigned int)hr, (unsigned long long)(GetTickCount64() - started));
    result = FAILED(hr);
done:
    if (reader) IUnknown_Release(reader);
    if (stream) IStream_Release(stream);
    if (factory) factory->vtbl->release(factory);
    FreeLibrary(module);
    CoUninitialize();
    return result;
}
