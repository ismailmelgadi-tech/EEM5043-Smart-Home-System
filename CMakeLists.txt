cmake_minimum_required(VERSION 3.5)

if(DEFINED ENV{RMAKER_PATH})
set(RMAKER_PATH $ENV{RMAKER_PATH})
else()
set(RMAKER_PATH ${CMAKE_CURRENT_LIST_DIR}/../..)
endif(DEFINED ENV{RMAKER_PATH})

# Add RainMaker components and other common application components
set(EXTRA_COMPONENT_DIRS ${RMAKER_PATH}/examples/common)

set(PROJECT_VER "1.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# استخدم اسم مشروع بسيط لتجنب الأخطاء
project(EEM5043-Smart-Home-System) 

# هذا الجزء يحل خطأ الربط (Undefined Reference Error) الذي كنا نواجهه:
link_libraries(-Wl,--wrap=esp_log_write -Wl,--wrap=esp_log_writev)