#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    static const WCHAR subkey[] =
        L"REGISTRY\\MACHINE\\Software\\Autodesk\\Inventor\\RegistryVersion31.0\\RemoveFiles";
    WCHAR path[MAX_PATH];
    HKEY hive = NULL, key = NULL;
    DWORD value = 0, type = 0, size = sizeof(value);
    LSTATUS status;

    if (argc != 2)
    {
        fprintf(stderr, "usage: regloadappkey.exe <Registry.dat>\n");
        return 2;
    }
    if (!MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, path, ARRAYSIZE(path)))
    {
        fprintf(stderr, "path conversion failed: %u\n", GetLastError());
        return 2;
    }

    status = RegLoadAppKeyW(path, &hive, KEY_READ, 0, 0);
    printf("RegLoadAppKeyW: %d handle=%p\n", status, hive);
    if (status) return 1;

    status = RegOpenKeyExW(hive, subkey, 0, KEY_READ, &key);
    printf("RegOpenKeyExW: %d handle=%p\n", status, key);
    if (status) { RegCloseKey(hive); return 1; }

    status = RegQueryValueExW(key, NULL, NULL, &type, (BYTE *)&value, &size);
    printf("RegQueryValueExW: %d type=%u size=%u value=%u\n",
           status, type, size, value);
    RegCloseKey(key);
    RegCloseKey(hive);
    return status || type != REG_DWORD || value != 1;
}
