cmake_minimum_required(VERSION 3.15)
project(3rdparty_libs)

#cv180x(64) cv181x(256)
set(TARGET_SOC "cv180x")

set(CVIMPI_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(CVIMPI_LIB_DIR "${CVIMPI_SDK_PATH}/lib")

# ive
set(IVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ive)
set(IVE_LIB ${IVE_PATH}/lib/)
if(TARGET_SOC STREQUAL "cv180x")  
    set(IVE_LIBS ${IVE_LIB}/libcvi_ive_tpu.so PARENT_SCOPE)
elseif(TARGET_SOC STREQUAL "cv181x")  
    set(IVE_LIBS ${IVE_LIB}/libcvi_ive.so PARENT_SCOPE)
endif()
set(IVE_INC ${IVE_PATH}/include PARENT_SCOPE)



# middleware
set(MW_PATH ${CMAKE_CURRENT_SOURCE_DIR}/middleware)
set(MW_LIB  ${MW_PATH}/v2/lib)
set(MW_AUDIO_LIBS   ${MW_LIB}/libcvi_audio.so
                    ${MW_LIB}/libcvi_vqe.so
                    ${MW_LIB}/libcvi_ssp.so
                    ${MW_LIB}/libtinyalsa.so
                    ${MW_LIB}/libcvi_VoiceEngine.so
                    ${MW_LIB}/libcvi_RES1.so
                    ${MW_LIB}/3rd/libcli.so	
                    PARENT_SCOPE		
)
set(MW_LIBS ${MW_LIB}/3rd/libini.so 
            ${MW_LIB}/libsns_full.so
            ${MW_LIB}/libsample.so
            ${MW_LIB}/libisp.so 
            ${MW_LIB}/libvdec.so
            ${MW_LIB}/libvenc.so 
            ${MW_LIB}/libawb.so 
            ${MW_LIB}/libae.so 
            ${MW_LIB}/libaf.so 
            ${MW_LIB}/libcvi_bin.so 
            ${MW_LIB}/libcvi_bin_isp.so 
            ${MW_LIB}/libmisc.so
            ${MW_LIB}/libisp_algo.so 
            ${MW_LIB}/libsys.so 
            ${MW_LIB}/libvpu.so
	    PARENT_SCOPE
)
set(MW_INC ${MW_PATH}/v2/include PARENT_SCOPE)
set(MW_LINUX_INC ${MW_PATH}/v2/include/linux PARENT_SCOPE)
set(MW_ISP_INC ${MW_PATH}/v2/include/isp/${TARGET_SOC} PARENT_SCOPE)


# rtsp
set(RTSP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rtsp)
set(RTSP_LIBS ${RTSP_PATH}/lib/libcvi_rtsp.so PARENT_SCOPE)
set(RTSP_INC ${RTSP_PATH}/include/cvi_rtsp PARENT_SCOPE)
#将这个变量的值传递回调用它的CMake作用域

# tpu
set(TPU_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tpu)
set(TPU_LIB ${TPU_PATH}/lib/)
set(TPU_LIBS ${TPU_LIB}/libcnpy.so
             ${TPU_LIB}/libcvikernel.so
             ${TPU_LIB}/libcvimath.so
             ${TPU_LIB}/libcviruntime.so
             ${TPU_LIB}/libz.so
             PARENT_SCOPE

)
set(TPU_INC ${TPU_PATH}/include PARENT_SCOPE)
#将这个变量的值传递回调用它的CMake作用域

set(OPENCV_PATH ${CMAKE_CURRENT_SOURCE_DIR}/opencv)
set(OPENCV_LIB ${OPENCV_PATH}/lib/)
set(OPENCV_LIBS ${OPENCV_LIB}/libopencv_core.so.3.2
		        ${OPENCV_LIB}/libopencv_imgcodecs.so.3.2
		        ${OPENCV_LIB}/libopencv_imgproc.so.3.2
		        PARENT_SCOPE
)
set(OPENCV_INC ${OPENCV_PATH}/include/opencv
	           ${OPENCV_PATH}/include/opencv2 
	           PARENT_SCOP
)



# stb
set(STB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/stb)
set(STB_INC ${STB_PATH}/include PARENT_SCOPE)

