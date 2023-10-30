#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include <imp_log.h>
#include <imp_common.h>
#include <imp_system.h>
#include <imp_framesource.h>
#include <imp_encoder.h>
#include <imp_osd.h>
#include <imp_utils.h>

#include "osd.h"

IMPRgnHandle *t31_osd_init(int grpNum)
{
    int ret;
    IMPRgnHandle *g_handle;
    IMPRgnHandle osdTime;

    g_handle = malloc(OSD_RGN_HANDER_MAX * sizeof(IMPRgnHandle));
    if (g_handle <= 0)
    {
        printf("Malloc fail\n");
        return NULL;
    }

    osdTime = IMP_OSD_CreateRgn(NULL);
    if (osdTime == INVHANDLE)
    {
        printf("IMP osd time create fail\n");
        return NULL;
    }

    ret = IMP_OSD_RegisterRgn(osdTime, grpNum, NULL);
    if (ret)
    {
        printf("IMP osd time register fail\n");
        return NULL;
    }

    IMPOSDRgnAttr attrTime;
    memset(&attrTime, 0, sizeof(IMPOSDRgnAttr));
    attrTime.type = OSD_REG_PIC;
    attrTime.rect.p0.x = OSD_TIME_X;
    attrTime.rect.p0.y = OSD_TIME_Y;
    attrTime.rect.p1.x = attrTime.rect.p0.x + 20 * OSD_REGION_WIDTH - 1;
    attrTime.rect.p1.y = attrTime.rect.p0.y + OSD_REGION_HEIGHT - 1;
    attrTime.fmt = PIX_FMT_BGRA;
    attrTime.data.picData.pData = NULL;

    ret = IMP_OSD_SetRgnAttr(osdTime, &attrTime);
    if (ret)
    {
        printf("IMP osd time set fail\n");
        return NULL;
    }

    IMPOSDGrpRgnAttr grAttrTime;
    if (IMP_OSD_GetGrpRgnAttr(osdTime, grpNum, &grAttrTime) < 0)
    {
        printf("IMP osd time get group error !\n");
        return NULL;
    }
    memset(&grAttrTime, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrTime.show = 0;

    grAttrTime.gAlphaEn = 1;
    grAttrTime.fgAlhpa = 0xff;
    grAttrTime.layer = 3;
    if (IMP_OSD_SetGrpRgnAttr(osdTime, grpNum, &grAttrTime) < 0)
    {
        printf("IMP osd time set group error !\n");
        return NULL;
    }

    ret = IMP_OSD_Start(grpNum);
    if (ret)
    {
        printf("IMP OSD Start error !\n");
        return NULL;
    }

    g_handle[OSD_RGN_HANDER_0] = osdTime;

    return g_handle;
}