#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include <imp_log.h>
#include <imp_common.h>
#include <imp_system.h>
#include <imp_framesource.h>
#include <imp_encoder.h>
#include <imp_osd.h>
#include <imp_utils.h>

#include "app.h"
#include "osd.h"
#include "bgramapinfo.h"

typedef struct
{
    bool bRun;
    pthread_t pthread;
} osd_handle_t;

typedef struct
{
    IMPRgnHandle osdHandle;
} t31_osd_context_t;

typedef struct
{
    int grpNum;
    osd_handle_t hdn;
} t31_osdHandle_t;

static t31_osdHandle_t this;

t31_osd_context_t *g_conf_handle[OSD_RGN_HANDER_MAX];

static t31_osd_context_t *create_osd_context_init(void)
{
    t31_osd_context_t *g_handle = NULL;
    g_handle = calloc(1, sizeof(t31_osd_context_t));
    if (g_handle == NULL)
    {
        printf("Can't calloc osd conf\n");
        return NULL;
    }

    return g_handle;
}

static void osd_release(IMPRgnHandle *g_handle)
{
    int ret;

    ret = IMP_OSD_DestroyGroup(this.grpNum);
    if (ret)
    {
        printf("IMP osd destroy group fail\n");
        return;
    }

    if (g_handle)
    {
        free(g_handle);
        g_handle = NULL;
    }
}

static int osdTime_init(IMPRgnHandle *g_handle, int grpNum)
{
    int ret;
    IMPRgnHandle osdTime;

    osdTime = IMP_OSD_CreateRgn(NULL);
    if (osdTime == INVHANDLE)
    {
        printf("IMP osd time create fail\n");
        return -1;
    }

    ret = IMP_OSD_RegisterRgn(osdTime, grpNum, NULL);
    if (ret)
    {
        printf("IMP osd time register fail\n");
        return -1;
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
        return -1;
    }

    IMPOSDGrpRgnAttr grAttrTime;
    if (IMP_OSD_GetGrpRgnAttr(osdTime, grpNum, &grAttrTime) < 0)
    {
        printf("IMP osd time get group error !\n");
        return -1;
    }
    memset(&grAttrTime, 0, sizeof(IMPOSDGrpRgnAttr));
    grAttrTime.show = 0;

    grAttrTime.gAlphaEn = 1;
    grAttrTime.fgAlhpa = 0xff;
    grAttrTime.layer = 3;
    if (IMP_OSD_SetGrpRgnAttr(osdTime, grpNum, &grAttrTime) < 0)
    {
        printf("IMP osd time set group error !\n");
        return -1;
    }

    ret = IMP_OSD_Start(grpNum);
    if (ret)
    {
        printf("IMP OSD Start error !\n");
        return -1;
    }

    g_handle = &osdTime;

    return 0;
}

static int osdTime_show(IMPRgnHandle osdTime, int grpNum)
{
    int ret;

    ret = IMP_OSD_ShowRgn(osdTime, grpNum, OSD_SHOW_ENABLE);
    if (ret)
    {
        printf("IMP osd show enable fail\n");
        return -1;
    }

    return 0;
}

static int osdTime_exit(IMPRgnHandle osdTime, int grpNum)
{
    int ret;

    ret = IMP_OSD_ShowRgn(osdTime, grpNum, OSD_SHOW_DISABLE);
    if (ret)
    {
        printf("IMP osd show disable fail\n");
        return -1;
    }

    ret = IMP_OSD_UnRegisterRgn(osdTime, grpNum);
    if (ret)
    {
        printf("IMP osd unregister fail\n");
        return 0;
    }

    IMP_OSD_DestroyRgn(osdTime);

    return 0;
}

static void *_osd(void *args)
{
    int i, j, ret = 0;
    char strDate[64];
    time_t currTime;
    struct tm *currDate;
    void *dateData = NULL;
    IMPOSDRgnAttrData rAttrData;
    t31_osd_context_t *g_handle = (t31_osd_context_t *)args;

    uint32_t *data = malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint32_t));
    if (data == NULL)
    {
        printf("valloc data error\n");
        return NULL;
    }

    ret = osdTime_show(g_handle[OSD_RGN_0].osdHandle, this.grpNum);
    if (ret)
    {
        printf("OSD time show fail\n");
        return NULL;
    }

    while (this.hdn.bRun)
    {
        int penpos_t = 0;
        int fontadv = 0;

        time(&currTime);
        currDate = localtime(&currTime);
        memset(strDate, 0, sizeof(strDate));
        strftime(strDate, sizeof(strDate), "%Y-%m-%d %I:%M:%s", currDate);
        for (i = 0; i < OSD_LETTER_NUM; i++)
        {
            switch (strDate[i])
            {
            case '0' ... '9':
                dateData = (void *)gBgramap[strDate[i] - '0'].pdata;
                fontadv = gBgramap[strDate[i] - '0'].width;
                penpos_t += gBgramap[strDate[i] - '0'].width;
                break;
            case '-':
                dateData = (void *)gBgramap[10].pdata;
                fontadv = gBgramap[10].width;
                penpos_t += gBgramap[10].width;
                break;
            case ' ':
                dateData = (void *)gBgramap[11].pdata;
                fontadv = gBgramap[11].width;
                penpos_t += gBgramap[11].width;
                break;
            case ':':
                dateData = (void *)gBgramap[12].pdata;
                fontadv = gBgramap[12].width;
                penpos_t += gBgramap[12].width;
                break;
            default:
                break;
            }

            for (j = 0; j < OSD_REGION_HEIGHT; j++)
            {
                memcpy((void *)((uint32_t *)data + j * OSD_LETTER_NUM * OSD_REGION_WIDTH + penpos_t),
                       (void *)((uint32_t *)dateData + j * fontadv), fontadv * sizeof(uint32_t));
            }
        }

        rAttrData.picData.pData = data;
        IMP_OSD_UpdateRgnAttrData(g_handle[OSD_RGN_0].osdHandle, &rAttrData);
        sleep(1);
    }

    return NULL;
}

int t31_osd_init()
{
    int i, ret;
    t31_osd_context_t *g_handle = NULL;

    for (i = 0; i < OSD_RGN_HANDER_MAX; i++)
    {
        g_handle = create_osd_context_init();
        if (g_handle == NULL)
        {
            printf("Can't allocate memory area\n");
            return -1;
        }

        g_conf_handle[i] = g_handle;
    }

    ret = IMP_OSD_CreateGroup(this.grpNum);
    if (ret)
    {
        printf("IMP osd create group\n");
        return -1;
    }

    ret = osdTime_init(&g_conf_handle[OSD_RGN_0]->osdHandle, this.grpNum);
    if (ret)
    {
        printf("Init osd time fail\n");
        return -1;
    }

    IMP_FrameSource_SetFrameDepth(0, 0);

    return 0;
}

int t31_osd_start()
{
    int ret;

    if (!this.hdn.bRun)
    {
        this.hdn.bRun = true;
        ret = pthread_create(&this.hdn.pthread, NULL, _osd, (void*)g_conf_handle);
        if (ret)
            return -1;
    }

    return 0;
}

void t31_osd_stop()
{
    if(this.hdn.bRun)
    {
        this.hdn.bRun = false;
        pthread_join(this.hdn.pthread, NULL);
    }
}

int t31_osd_release()
{
    
}
