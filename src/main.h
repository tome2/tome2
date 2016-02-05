#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int main_real(int argc, char *argv[], char const *platform_sys, int (*init_platform)(int, char *[]), char const *platform_usage);

#ifdef __cplusplus
} // extern "C"
#endif
