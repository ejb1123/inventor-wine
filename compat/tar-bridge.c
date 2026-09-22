/* Minimal Wine bridge for the Windows tar invocation used by Autodesk's
 * Electrical Catalog custom action. Paths are translated by Wine, and execv
 * forwards arguments without a shell. Unsupported invocations fail closed.
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#ifndef BSDTAR_PATH
#error BSDTAR_PATH must identify the packaged native bsdtar executable
#endif
int main(int argc, char **argv)
{
    char *paths[2];
    WCHAR wide[32768];
    char * (CDECL *unix_name)(LPCWSTR);
    if (argc != 6 || strcmp(argv[1], "-xf") || strcmp(argv[3], "-C") || strcmp(argv[5], "--keep-newer-files")) {
        fprintf(stderr,"tar bridge: unsupported arguments\n"); return 2;
    }
    unix_name=(void *)GetProcAddress(GetModuleHandleA("kernel32"),"wine_get_unix_file_name");
    if (!unix_name) return 2;
    for (int i=0;i<2;i++) {
        if (!MultiByteToWideChar(CP_UNIXCP,0,argv[2+i*2],-1,wide,32768)) return 2;
        paths[i]=unix_name(wide);
        if (!paths[i]) return 2;
    }
    char *args[]={"bsdtar","-xf",paths[0],"-C",paths[1],"--keep-newer-files",NULL};
    /* Keep the Wine process alive until extraction completes. Replacing it
     * with execv would close the server connection and let the MSI continue
     * before bsdtar finishes writing its output. */
    pid_t child = fork();
    if (child < 0) return 2;
    if (!child) {
        execv(BSDTAR_PATH,args);
        _exit(127);
    }
    int status;
    while (waitpid(child,&status,0) < 0) {
        if (errno != EINTR) return 2;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 2;
}
