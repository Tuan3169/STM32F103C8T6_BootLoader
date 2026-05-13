/*****************************************************************************************************************
 * @Filename: ProjectConfig.h
 * @Author: Đinh Văn Tuấn
 * @Date 2026-05-02
 * @Version: 1.0
 * @Description: Config information for project
 * 
 * Copyright (c) 2026 []. All rights reserved.
 * Licensed under the MIT License.
 ****************************************************************************************************************/


#ifndef _ProjectConfig_H
#define _ProjectConfig_H


//################################################# INCLUDE HEARDER #############################################


//#################################################     DEFINE    ###############################################
#define BOOTLOADER_START_ADDRESS            0x08000000
#define APP_START_ADDRESS                   0x08004000
#define APP_OTA_START_ADDRESS               0x0800A000

// Application image layout (App + UserData)
#define APP_IMAGE_SIZE                      0x00007000
#define APP_HASH_OFFSET                      (APP_IMAGE_SIZE - 4U)
#define APP_CRC_OFFSET                       (APP_IMAGE_SIZE - 8U)
#define APP_HASH_ADDRESS                     (APP_START_ADDRESS + APP_HASH_OFFSET)
#define APP_CRC_ADDRESS                      (APP_START_ADDRESS + APP_CRC_OFFSET)

// OTA flag definitions
#define OTA_FLAG_ADDRESS                    0x0800FC00  // Nằm trong USER_DATA section
#define OTA_FLAG_MAGIC                      0xDEADBEEF
#define BOOT_FLAG_MAGIC                     0x00000000  // Flag xóa = reset về bootloader/app

//#################################################     TYPEDEF   ###############################################


//#################################################     VARIABLE  ###############################################


//#################################################     FUNCTION  ###############################################


#endif /* _ProjectConfig_H */