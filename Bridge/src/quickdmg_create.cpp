#include "../include/quickdmg_kit.h"
#include "../include/dsstore_builder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static int run_command(const char *cmd) {
    return system(cmd);
}

extern "C" int quickdmg_create_dmg(const quickdmg_create_config *config,
                        const char *output_path,
                        quickdmg_progress_cb prog_cb,
                        void *user_data)
{
    if (!config || !output_path || !config->source_dir || !config->volume_name) {
        return QUICKDMG_ERROR_INVALID_ARG;
    }

    char tmp_dmg[1024];
    snprintf(tmp_dmg, sizeof(tmp_dmg), "/tmp/quickdmg_build_%d.dmg", getpid());
    unlink(tmp_dmg);

    char cmd[2048];

    // 1. Create a large enough read-write DMG
    snprintf(cmd, sizeof(cmd), "hdiutil create -size 500m -fs HFS+ -volname \"%s\" \"%s\" -ov",
             config->volume_name, tmp_dmg);
    if (run_command(cmd) != 0) {
        return QUICKDMG_ERROR_CREATE;
    }

    // 2. Attach the DMG
    char mount_point[1024];
    snprintf(mount_point, sizeof(mount_point), "/Volumes/%s", config->volume_name);
    
    // Attempt detach just in case it's already mounted
    snprintf(cmd, sizeof(cmd), "hdiutil detach \"%s\" -force 2>/dev/null", mount_point);
    run_command(cmd);

    snprintf(cmd, sizeof(cmd), "hdiutil attach \"%s\" -mountpoint \"%s\" -nobrowse", tmp_dmg, mount_point);
    if (run_command(cmd) != 0) {
        unlink(tmp_dmg);
        return QUICKDMG_ERROR_CREATE;
    }

    // 3. File copy
    snprintf(cmd, sizeof(cmd), "cp -R \"%s\"/* \"%s\"/", config->source_dir, mount_point);
    if (run_command(cmd) != 0) {
        snprintf(cmd, sizeof(cmd), "hdiutil detach \"%s\" -force", mount_point);
        run_command(cmd);
        unlink(tmp_dmg);
        return QUICKDMG_ERROR_CREATE;
    }

    if (config->background_image) {
        char bg_dir[1024];
        snprintf(bg_dir, sizeof(bg_dir), "%s/.background", mount_point);
        mkdir(bg_dir, 0755);
        snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s/\"", config->background_image, bg_dir);
        run_command(cmd);
    }

    // 4. Generate .DS_Store
    dsstore_config_t ds_conf = {0};
    ds_conf.window_x = 100;
    ds_conf.window_y = 100;
    ds_conf.window_w = config->window_width ? config->window_width : 600;
    ds_conf.window_h = config->window_height ? config->window_height : 400;
    ds_conf.icon_size = config->icon_size ? config->icon_size : 128;
    ds_conf.background_image_path = config->background_image;

    dsstore_icon_entry_t *icons = (dsstore_icon_entry_t *)calloc(config->num_icon_positions, sizeof(dsstore_icon_entry_t));
    for (uint32_t i = 0; i < config->num_icon_positions; i++) {
        icons[i].filename = config->icon_positions[i].filename;
        icons[i].pos.x = config->icon_positions[i].x;
        icons[i].pos.y = config->icon_positions[i].y;
    }
    ds_conf.icons = icons;
    ds_conf.icon_count = config->num_icon_positions;

    size_t ds_size = 0;
    uint8_t *ds_data = dsstore_build(&ds_conf, &ds_size);
    if (ds_data) {
        char ds_path[1024];
        snprintf(ds_path, sizeof(ds_path), "%s/.DS_Store", mount_point);
        FILE *f = fopen(ds_path, "wb");
        if (f) {
            fwrite(ds_data, 1, ds_size, f);
            fclose(f);
        }
        free(ds_data);
    }
    free(icons);

    // 5. Detach
    snprintf(cmd, sizeof(cmd), "hdiutil detach \"%s\" -force", mount_point);
    run_command(cmd);

    // 6. Convert to compressed DMG
    unlink(output_path);
    snprintf(cmd, sizeof(cmd), "hdiutil convert \"%s\" -format UDZO -imagekey zlib-level=9 -o \"%s\"", tmp_dmg, output_path);
    if (run_command(cmd) != 0) {
        unlink(tmp_dmg);
        return QUICKDMG_ERROR_CREATE;
    }

    unlink(tmp_dmg);
    return QUICKDMG_OK;
}
