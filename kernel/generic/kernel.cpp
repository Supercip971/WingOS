#include <com.h>
#include <device/pit.h>
#include <device/rtc.h>
#include <filesystem/file_system.h>
#include <kernel.h>
#include <kernel_service/kernel_service.h>
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
    load_kernel_service();

    launch_programm("init_fs/wstart.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/graphic_service.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/background.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test2.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test.exe", main_fs_system::the()->main_fs());
    log("kernel", LOG_INFO, "==== KERNEL STARTED ====");

    log("kernel", LOG_INFO, ":::       ::: ::::::::::: ::::    :::  ::::::::         ::::::::   ::::::::");
    log("kernel", LOG_INFO, ":+:       :+:     :+:     :+:+:   :+: :+:    :+:       :+:    :+: :+:    :+:");
    log("kernel", LOG_INFO, "+:+       +:+     +:+     :+:+:+  +:+ +:+              +:+    +:+ +:+        ");
    log("kernel", LOG_INFO, "+#+  +:+  +#+     +#+     +#+ +:+ +#+ :#:              +#+    +:+ +#++:++#++ ");
    log("kernel", LOG_INFO, "+#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+   +#+#       +#+    +#+        +#+ ");
    log("kernel", LOG_INFO, " #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+#       #+#    #+# #+#    #+# ");
    log("kernel", LOG_INFO, "  ###   ###   ########### ###    ####  ########         ########   ########  ");

    log("kernel", LOG_INFO, "kernel started with: {} memory available ", get_total_memory() * PAGE_SIZE);
    log("kernel", LOG_INFO, "memory used: {}", get_used_memory() * PAGE_SIZE);
    //  dump_process();
    while (true)
    {
    }
}
