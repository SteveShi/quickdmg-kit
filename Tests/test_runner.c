#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Bridge/include/quickdmg_kit.h"

static const char *g_password = NULL;

static bool password_callback(char *password_buf, size_t max_len, void *user_data) {
    printf("[QuickdmgKit Test] Password callback requested! Giving: %s\n", g_password);
    if (!g_password) return false;
    strncpy(password_buf, g_password, max_len - 1);
    password_buf[max_len - 1] = 0;
    return true;
}

static void progress_callback(uint64_t completed_bytes, uint64_t total_bytes, const char *current_path, void *user_data) {
    if (total_bytes > 0) {
        double pct = (double)completed_bytes * 100.0 / (double)total_bytes;
        printf("\r[Extracting] %5.1f%% (%llu / %llu bytes) %s", pct, completed_bytes, total_bytes, current_path ? current_path : "");
        fflush(stdout);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    const char *dmg_path = argv[1];
    const char *output_dir = (argc >= 3) ? argv[2] : NULL;
    if (argc >= 4) g_password = argv[3];

    quickdmg_archive *ar = NULL;
    int res = quickdmg_open(dmg_path, password_callback, NULL, &ar);
    printf("quickdmg_open result: %d\n", res);
    if (res == QUICKDMG_OK) {
        uint32_t count = 0;
        quickdmg_get_item_count(ar, &count);
        printf("Item count: %u\n", count);
        for (uint32_t i = 0; i < count; i++) {
            quickdmg_item_info info;
            quickdmg_get_item_info(ar, i, &info);
            printf("  Item %u: %s (%llu bytes)\n", i, info.path, info.size);
        }
        if (output_dir) {
            quickdmg_extract_all(ar, output_dir, progress_callback, NULL);
            printf("\nExtraction done!\n");
        }
        quickdmg_close(ar);
    }
    return 0;
}
