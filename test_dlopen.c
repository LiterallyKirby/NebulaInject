#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <jni.h>

static jmp_buf jump_buffer;
void segfault_handler(int signum) {
    printf("Segmentation fault caught! Signal: %d\n", signum);
    longjmp(jump_buffer, 1);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <library_path>\n", argv[0]);
        return 1;
    }

    const char* lib_path = argv[1];
    printf("Attempting to load: %s\n", lib_path);

    signal(SIGSEGV, segfault_handler);

    if (setjmp(jump_buffer) != 0) {
        printf("Exiting after segfault.\n");
        return 1;
    }

    dlerror();
    void* handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    printf("Library loaded successfully!\n");

    void* sym = dlsym(handle, "JNI_OnLoad");
    if (!sym) {
        printf("Symbol JNI_OnLoad not found: %s\n", dlerror());
    } else {
        printf("Found JNI_OnLoad, calling...\n");
        typedef jint (*JNI_OnLoad_t)(JavaVM*, void*);
        ((JNI_OnLoad_t)sym)(NULL, NULL); // C style NULL
        printf("JNI_OnLoad call finished.\n");
    }

    printf("Closing library...\n");
    dlclose(handle);
    printf("Library closed.\n");

    return 0;
}
