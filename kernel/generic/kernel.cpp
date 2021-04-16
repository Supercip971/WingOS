#include <com.h>
#include <device/pit.h>
#include <device/rtc.h>
#include <filesystem/file_system.h>
#include <kernel.h>
#include <logging.h>
#include <physical.h>
#include <process.h>
#include <programm_launcher.h>
#include <stdint.h>
#include <stivale_struct.h>
#include <utility.h>
/*
    pour le moment tout ce qui est ici est un test
    for the moment everything here is for test
*/
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
        log("kernel", LOG_INFO) << "hey hey";
    }
}
void _start(stivale_struct *bootloader_data)
{
    main_fs_system::the()->init_file_system();

    log("kernel", LOG_INFO, "==== KERNEL STARTED ====");

    show_art();

    log("kernel", LOG_INFO, "kernel started with: {} memory available ", get_total_memory() * PAGE_SIZE);
    log("kernel", LOG_INFO, "memory used: {}", get_used_memory() * PAGE_SIZE);
    //  dump_process();
    launch_programm("initfs/graphic_service.exe", main_fs_system::the()->main_fs(), 0, nullptr);
    sleep(1);
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
