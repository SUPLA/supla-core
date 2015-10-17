/*
 ============================================================================
 Name        : supla_client-jni.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <jni.h>
#include "proto.h"
#include "supla-client.h"

void Java_org_supla_android_CfgInit(JNIEnv* env, jobject thiz, jobject cfg) {

      TSuplaClientCfg sclient_cfg;
    
      jclass jcs = (*env)->GetObjectClass(env, cfg);
    
      if ( jcs ) {
          supla_client_cfginit(&sclient_cfg);
      };
    
     
    
}

