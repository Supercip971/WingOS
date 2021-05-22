#include <device/debug/com.h>
#include <device/time/pit.h>
#include <device/time/rtc.h>
#include <filesystem/file_system.h>
#include <kernel.h>
#include <logging.h>
#include <physical.h>
#include <proc/process.h>
#include <programm_launcher.h>
#include <stdint.h>
#include <stivale_struct.h>
#include <utility.h>

void show_art()
{
    log("kernel", LOG_INFO, ":::       ::: ::::::::::: ::::    :::  ::::::::         ::::::::   ::::::::");
    log("kernel", LOG_INFO, ":+:       :+:     :+:     :+:+:   :+: :+:    :+:       :+:    :+: :+:    :+:");
    log("kernel", LOG_INFO, "+:+       +:+     +:+     :+:+:+  +:+ +:+              +:+    +:+ +:+        ");
    log("kernel", LOG_INFO, "+#+  +:+  +#+     +#+     +#+ +:+ +#+ :#:              +#+    +:+ +#++:++#++ ");
    log("kernel", LOG_INFO, "+#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+   +#+#       +#+    +#+        +#+ ");
    log("kernel", LOG_INFO, " #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+#       #+#    #+# #+#    #+# ");
    log("kernel", LOG_INFO, "  ###   ###   ########### ###    ####  ########         ########   ########  ");
}

void test()
{
    while (true)
    {
        sleep(2);
        log("kernel", LOG_INFO, "hey hey");
    }
}

void _start(stivale_struct *bootloader_data)
{
    main_fs_system::the()->init_file_system();
    launch_module("initfs/ps2_mouse_module.module", main_fs_system::the()->main_fs(), 0, nullptr);
    sleep(500);

    launch_module("initfs/ps2_keyboard_module.module", main_fs_system::the()->main_fs(), 0, nullptr);
    sleep(500);
    log("kernel", LOG_INFO, "==== KERNEL STARTED ====");

    show_art();

    log("kernel", LOG_INFO, "kernel started with: {} memory available ", get_total_memory() * PAGE_SIZE);
    log("kernel", LOG_INFO, "memory used: {}", get_used_memory() * PAGE_SIZE);
    //  dump_process();
    launch_programm("initfs/graphic_service.exe", main_fs_system::the()->main_fs(), 0, nullptr);
    sleep(10);
    launch_programm("initfs/background.exe", main_fs_system::the()->main_fs(), 0, nullptr);
    launch_programm("initfs/test2.exe", main_fs_system::the()->main_fs(), 0, nullptr);

    launch_programm("initfs/test.exe", main_fs_system::the()->main_fs(), 0, nullptr);
    launch_programm("initfs/test.exe", main_fs_system::the()->main_fs(), 0, nullptr);
    const char *argv[] = {"hello", "world"};
    launch_programm("initfs/test.exe", main_fs_system::the()->main_fs(), 2, argv);
    while (true)
    {
        sleep(1000);
        log("kernel", LOG_INFO, "memory used: {}", get_used_memory() * PAGE_SIZE);
    }
}
