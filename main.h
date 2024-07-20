// CMakeProject2.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
typedef enum signed_file_type_t {
    SFT_INVALID = 0,    // 显式的无效定义.
    /*-------------------------------------------------------*/
    SFT_NOT_IDENTIFIED, // "未被标识". 
    /*-------------------------------------------------------*/
    SFT_APK,        // apk 文件.
    SFT_PKG,        // pgk 文件. 
    /*-------------------------------------------------------*/
}   signed_file_type_t;
// TODO: 在此处引用程序需要的其他标头。
