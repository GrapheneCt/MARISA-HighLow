cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
    else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
    endif()
endif()

project(MARISA-HIGHLOW)
include("${DOLCESDK}/share/dolce.cmake" REQUIRED)
set(VITA_APP_NAME "MARISA HighLow")
set(VITA_TITLEID  "MARISAHIL")
set(VITA_VERSION  "02.00")
set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=9")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -fno-builtin")

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
	)

add_executable(${PROJECT_NAME}
	src/main.c
	src/ctrl.c
	src/graphics.c
	src/sound.c
	src/psarc.c
	src/misc.c
	)

target_link_libraries(${PROJECT_NAME}
	vitaSAS_stub
	vita2d_sys_stub
	SceGxm_stub
	SceLibKernel_stub
	SceSysmem_stub
	SceKernelThreadMgr_stub
	SceProcessMgr_stub
	SceAppMgr_stub 
	ScePvf_stub 
	SceDisplay_stub 
	SceSysmodule_stub 
	SceCtrl_stub
	SceTouch_stub 
	SceAudio_stub 
	SceIofilemgr_stub
	SceCodecEngine_stub 
	SceAudiodec_stub 
	SceSas_stub
	SceFios2_stub
	SceSystemGesture_stub
	SceAppUtil_stub
	SceDbg_stub
	SceLibc_stub
	gcc
    )
	
set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-nostdlib"
)
	
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
  COMMAND dolce-mkbootparam -main-budget 0x80 -app-type 0x08 ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

dolce_create_self(eboot.bin ${PROJECT_NAME} UNSAFE BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin)

dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
	FILE sce_sys/icon0.png sce_sys/icon0.png
	   sce_sys/pic0.png sce_sys/pic0.png
       sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
       sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
       sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
	   sce_sys/manual/001.png sce_sys/manual/001.png
	   sce_sys/manual/002.png sce_sys/manual/002.png
	   sce_sys/manual/003.png sce_sys/manual/003.png
	   sce_sys/manual/004.png sce_sys/manual/004.png
	   font/07LogoTypeGothic-CondenseLatin.ttf font/07LogoTypeGothic-CondenseLatin.ttf
	   gamedata.psarc gamedata.psarc
	   vita2d_sys.suprx module/vita2d_sys.suprx
	   vitaSAS.suprx module/vitaSAS.suprx
    )
