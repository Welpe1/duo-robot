#include "cvi_tdl.h"
#include "cvi_tdl_app.h"
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include "middleware.h"

static volatile bool bExit = false;

static void SampleHandleSig(int signo) {
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  printf("handle signal, signo: %d\n", signo);
  if (SIGINT == signo || SIGTERM == signo) {
    bExit = true;
  }
}

int main(int argc, char *argv[]) {

  signal(SIGINT, SampleHandleSig);
  signal(SIGTERM, SampleHandleSig);

  int ret=0;
  SAMPLE_TDL_MW_CONTEXT stMWContext = {0};
  ret = MiddleWare_Init(&stMWContext);
  if (ret != CVI_TDL_SUCCESS) {
    printf("middleware init failed with %#x!\n", ret);
    goto CLEANUP_SYSTEM;
  }

  cvitdl_handle_t cvi_handle = NULL;
  cvitdl_service_handle_t cvi_service_handle = NULL;

  if(CVI_TDL_CreateHandle2(&cvi_handle,1,0)) goto CLEANUP_SYSTEM;
  if(CVI_TDL_SetVpssTimeout(cvi_handle, 1000)) goto CLEANUP_SYSTEM;
  if(CVI_TDL_SetVBPool(cvi_handle, 0, 2)) goto CLEANUP_SYSTEM;
  if(CVI_TDL_Service_CreateHandle(&cvi_service_handle, cvi_handle)) goto CLEANUP_SYSTEM2;

  VIDEO_FRAME_INFO_S Frame;
  while(bExit == false)
  {
    
    ret = CVI_VPSS_GetChnFrame(0, 0, &Frame, 2000);  
    if (ret) {  
      printf("CVI_VPSS_GetChnFrame chn0 failed\n");
      goto ERROR;
    }
    ret = SAMPLE_TDL_Send_Frame_RTSP(&Frame,&stMWContext);
    if (ret) {
      printf("RTSP Send Frame NG\n");
      goto ERROR;
    }
    ERROR:   
      CVI_VPSS_ReleaseChnFrame(0, 0, &Frame);  
      if(ret) bExit = true;   
  }

 
CLEANUP_SYSTEM2:
  CVI_TDL_Service_DestroyHandle(cvi_service_handle);
CLEANUP_SYSTEM:
  CVI_TDL_DestroyHandle(cvi_handle);
  SAMPLE_TDL_Destroy_MW(&stMWContext);
  CVI_SYS_Exit();
  CVI_VB_Exit();
}






