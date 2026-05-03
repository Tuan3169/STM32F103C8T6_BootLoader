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
#define APP_START_ADDRESS                   0x08001000
#define APP_OTA_START_ADDRESS               0x08008800
#define OTA_FLAG_ADDRESS                    0x0800FC00  // USER_DATA section: flag để bootloader biết nhảy sang OTA
#define OTA_FLAG_MAGIC                      0xDEADBEEF

//#################################################     TYPEDEF   ###############################################


//#################################################     VARIABLE  ###############################################


//#################################################     FUNCTION  ###############################################


#endif /* _ProjectConfig_H */