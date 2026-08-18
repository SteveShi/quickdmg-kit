#ifndef QUICKDMG_KIT_H
#define QUICKDMG_KIT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque archive handle
typedef struct quickdmg_archive quickdmg_archive;

// Item metadata structure
typedef struct {
    const char *path;            // Item relative path within archive
    uint64_t size;               // Uncompressed size in bytes
    uint64_t pack_size;          // Compressed size in bytes
    bool is_dir;                 // True if directory
    bool is_symlink;             // True if symbolic link
    const char *symlink_target;  // Target path if is_symlink is true
    uint32_t posix_attrib;       // POSIX file permissions & mode bits
    int64_t mtime;               // Modification time (Unix timestamp in seconds)
} quickdmg_item_info;

// Callback function definitions
typedef void (*quickdmg_progress_cb)(uint64_t completed_bytes, uint64_t total_bytes, const char *current_path, void *user_data);

typedef bool (*quickdmg_password_cb)(char *password_buf, size_t max_len, void *user_data);

#define QUICKDMG_OK                 0
#define QUICKDMG_ERROR_OPEN        -1
#define QUICKDMG_ERROR_PASSWORD    -2
#define QUICKDMG_ERROR_CORRUPT     -3
#define QUICKDMG_ERROR_EXTRACT     -4
#define QUICKDMG_ERROR_INVALID_ARG -5
#define QUICKDMG_ERROR_UNSUPPORTED -6

int quickdmg_open(const char *path,
                  quickdmg_password_cb pass_cb,
                  void *user_data,
                  quickdmg_archive **out_archive);

int quickdmg_get_item_count(quickdmg_archive *ar, uint32_t *count);

int quickdmg_get_item_info(quickdmg_archive *ar, uint32_t index, quickdmg_item_info *info);

int quickdmg_extract_all(quickdmg_archive *ar,
                        const char *output_dir,
                        quickdmg_progress_cb prog_cb,
                        void *user_data);

int quickdmg_extract_items(quickdmg_archive *ar,
                          const uint32_t *indices,
                          uint32_t num_indices,
                          const char *output_dir,
                          quickdmg_progress_cb prog_cb,
                          void *user_data);

void quickdmg_close(quickdmg_archive *ar);

#ifdef __cplusplus
}
#endif

#endif // QUICKDMG_KIT_H
