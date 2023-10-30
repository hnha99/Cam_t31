#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>

#include <imp_system.h>
#include <imp_isp.h>
#include <imp_common.h>
#include <imp_framesource.h>
#include <imp_encoder.h>
#include <imp_log.h>

#include "app.h"
#include "video.h"

typedef struct
{
    pthread_t pthread;
    bool bRun;
} video_handle_t;

typedef struct
{
    int channel;
    video_handle_t hnd;
    pthread_mutex_t mutex;
} t31_video_ctx_t;

static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_QUALITY;
static t31_video_ctx_t *g_video_context[MAX_STREAM_NUM];

static chn_conf_t chn_conf[MAX_STREAM_NUM] = {
    {
        .index = CHANNEL_0,
        .enable = CHN0_EN,
        .profile = IMP_ENC_PROFILE_AVC_MAIN,
        .fs_chn_attr = {
            .pixFmt = PIX_FMT_NV12,
            .outFrmRateNum = 25,
            .outFrmRateDen = 1,
            .nrVBs = 2,
            .type = FS_PHY_CHANNEL,
            .picWidth = SENSOR_WIDTH,
            .picHeight = SENSOR_HEIGHT,
        },
        .framesource_chn = {DEV_ID_FS, CHANNEL_0, 0},
        .imp_encoder = {DEV_ID_ENC, CHANNEL_0, 0},
    },
    {
        .index = CHANNEL_1,
        .enable = CHN1_EN,
        .profile = IMP_ENC_PROFILE_AVC_MAIN,
        .fs_chn_attr = {
            .pixFmt = PIX_FMT_NV12,
            .outFrmRateNum = 25,
            .outFrmRateDen = 1,
            .nrVBs = 2,
            .type = FS_PHY_CHANNEL,
            .picWidth = SENSOR_WIDTH,
            .picHeight = SENSOR_HEIGHT,
        },
        .framesource_chn = {DEV_ID_FS, CHANNEL_1, 0},
        .imp_encoder = {DEV_ID_ENC, CHANNEL_1, 0},
    }};

extern int IMP_OSD_SetPoolSize(int size);
IMPSensorInfo sensor_info;

static int save_stream(FILE *fd, IMPEncoderStream *stream)
{
    int ret = 0, i = 0, nr_pack = stream->packCount;

    for (i = 0; i < nr_pack; i++)
    {
        IMPEncoderPack *pack = &stream->pack[i];
        if (pack->length)
        {
            uint32_t remSize = stream->streamSize - pack->offset;
            if (remSize < pack->length)
            {
                ret = fwrite((void *)(stream->virAddr + pack->offset), 1, remSize, fd);
                if (ret != remSize)
                    return -1;

                ret = fwrite((void *)stream->virAddr, 1, pack->length - remSize, fd);
                if (ret != (pack->length - remSize))
                    return -1;
            }
            else
            {
                ret = fwrite((void *)(stream->virAddr + pack->offset), 1, pack->length, fd);
                if (ret != pack->length)
                    return -1;
            }
        }
    }

    return 0;
}

t31_video_ctx_t *create_video_context(void)
{
    t31_video_ctx_t *ctx = NULL;
    ctx = calloc(1, sizeof(t31_video_ctx_t));
    if (ctx == NULL)
        return (void *)-1;

    pthread_mutex_init(&ctx->mutex, NULL);

    return ctx;
}

void release_context(t31_video_ctx_t *ctx)
{
    if (ctx)
    {
        pthread_mutex_destroy(&ctx->mutex);
        free(ctx);
    }
}

int t31_system_init()
{
    int ret = 0;

    // IMP_OSD_SetPoolSize(512 * 1024);
    memset(&sensor_info, 0, sizeof(IMPSensorInfo));
    memcpy(sensor_info.name, SENSOR_NAME, sizeof(SENSOR_NAME));
    sensor_info.cbus_type = SENSOR_CUBS_TYPE;
    memcpy(sensor_info.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
    sensor_info.i2c.addr = SENSOR_I2C_ADDR;

    printf("IMP system init start\n");

    ret = IMP_ISP_Open();
    if (ret < 0)
    {
        printf("Failed to open ISP\n");
        return -1;
    }

    ret = IMP_ISP_AddSensor(&sensor_info);
    if (ret < 0)
    {
        printf("Failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_EnableSensor();
    if (ret < 0)
    {
        printf("Failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_System_Init();
    if (ret < 0)
    {
        printf("IMP_System_Init failed\n");
        return -1;
    }

    /* enable turning, to debug graphics */
    ret = IMP_ISP_EnableTuning();
    if (ret < 0)
    {
        printf("IMP_ISP_EnableTuning failed\n");
        return -1;
    }

    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(128);

#if 1
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0)
    {
        printf("Failed to set running mode\n");
        return -1;
    }
#endif

#if 0
    ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FRAME_RATE_NUM, SENSOR_FRAME_RATE_DEN);
    if (ret < 0)
    {
        printf("Failed to set sensor fps\n");
        return -1;
    }
#endif

    printf("IMP system init success\n");

    return 0;
}

int t31_system_exit()
{
    int ret = 0;

    IMP_System_Exit();

    ret = IMP_ISP_DisableSensor();
    if (ret < 0)
    {
        printf("Failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_ISP_DelSensor(&sensor_info);
    if (ret < 0)
    {
        printf("Failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_DisableTuning();
    if (ret < 0)
    {
        printf("IMP ISP DisableTuning failed\n");
        return -1;
    }

    if (IMP_ISP_Close())
    {
        printf("Failed to open ISP\n");
        return -1;
    }

    printf("IMP system exit success\n");

    return 0;
}

int t31_fs_init()
{
    int i, ret = 0;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            ret = IMP_FrameSource_CreateChn(chn_conf[i].index, &chn_conf[i].fs_chn_attr);
            if (ret)
            {
                printf("IMP FS create channel fail\n");
                return -1;
            }

            ret = IMP_FrameSource_SetChnAttr(chn_conf[i].index, &chn_conf[i].fs_chn_attr);
            if (ret)
            {
                printf("IMP FS set channel fail\n");
                return -1;
            }
        }
    }

    return 0;
}

int t31_fs_exit()
{
    int ret, i = 0;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            ret = IMP_FrameSource_DestroyChn(chn_conf[i].index);
            if (ret)
            {
                printf("IMP FS destroy channel fail\n");
            }
        }
    }

    return 0;
}

int t31_encode_init()
{
    int i, ret = 0;
    IMPEncoderChnAttr channel_attr;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        int group = chn_conf[i].index;

        ret = IMP_Encoder_CreateGroup(group);
        if (ret < 0)
        {
            printf("IMP encode create group fail\n");
            return -1;
        }

        ret = IMP_Encoder_CreateChn(chn_conf[i].index, &channel_attr);
        if (ret < 0)
        {
            printf("IMP encode create channel fail\n");
            return -1;
        }

        ret = IMP_Encoder_RegisterChn(group, chn_conf[i].index);
        if (ret < 0)
        {
            printf("IMP encode register channel fail\n");
            return -1;
        }
    }

    return 0;
}

int t31_encode_exit()
{
    int i, ret = 0;
    IMPEncoderChnStat chnStat;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        memset(&chnStat, 0, sizeof(IMPEncoderChnStat));
        if (chn_conf[i].enable)
        {
            ret = IMP_Encoder_Query(chn_conf[i].index, &chnStat);
            if (ret)
            {
                printf("IMP encode query fail\n");
                return -1;
            }

            if (chnStat.registered)
            {
                ret = IMP_Encoder_UnRegisterChn(chn_conf[i].index);
                if (ret)
                {
                    printf("IMP encode unregister channel fail\n");
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chn_conf[i].index);
                if (ret)
                {
                    printf("IMP encode destroy channel fail\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}

int t31_encode_setParam(int channel, IMPEncoderChnAttr *channel_attr, size_pic_t size, int fps, int BitRate)
{
    int ret = 0;
    int outFramRateDen = 1;

    memset(channel_attr, 0, sizeof(IMPEncoderChnAttr));
    ret = IMP_Encoder_SetDefaultParam(channel_attr, chn_conf[channel].profile, S_RC_METHOD,
                                      size.width, size.heigth, fps, outFramRateDen,
                                      fps * 2 / outFramRateDen, 2,
                                      (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1, BitRate);
    if (ret)
    {
        printf("IMP encode set param fail\n");
        return -1;
    }

    return 0;
}

int t31_fs_streamon()
{
    int i, ret = 0;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            ret = IMP_FrameSource_EnableChn(chn_conf[i].index);
            if (ret)
            {
                printf("IMP fs enable channel fail\n");
                return -1;
            }
        }
    }

    return 0;
}

int t31_fs_streamoff()
{
    int i, ret = 0;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            ret = IMP_FrameSource_DisableChn(chn_conf[i].index);
            if (ret)
            {
                printf("IMP fs disable channel fail\n");
                return -1;
            }
        }
    }

    return 0;
}

static void *_s(void *args)
{
    int chn, ret = 0;
    int tmp, chnNum, encType = 0;
    char file_path[128];
    t31_video_ctx_t *ctx = (t31_video_ctx_t *)args;

    chn = ctx->channel;
    printf("Start video channel %d\n", chn);

    if (chn_conf[chn].profile == IMP_ENC_PROFILE_JPEG)
    {
        tmp = (((chn_conf[chn].profile >> 24) << 16) | (4 + chn_conf[chn].index));
    }
    else
    {
        tmp = (((chn_conf[chn].profile >> 24) << 16) | chn_conf[chn].index);
    }

    chnNum = tmp & 0xffff;
    encType = (tmp >> 16) & 0xffff;

    ret = IMP_Encoder_StartRecvPic(chnNum);
    if (ret)
    {
        printf("IMP start recvpic fail\n");
        return NULL;
    }

    sprintf(file_path, "%s/video-%d.%s", T31_STREAM_PATH, chnNum,
            (encType == IMP_ENC_TYPE_AVC) ? "h264" : ((encType == IMP_ENC_TYPE_HEVC) ? "h265" : "jpeg"));

    FILE *fd = fopen(file_path, "a+");
    if (fd)
    {
        printf("Open file fail\n");
        return NULL;
    }

    while (ctx->hnd.bRun)
    {
        ret = IMP_Encoder_PollingStream(chnNum, 1000);
        if (ret < 0)
        {
            printf("IMP encode polling stream timeout\n");
            return NULL;
        }

        IMPEncoderStream stream;
        ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
        if (ret < 0)
        {
            printf("IMP encode get stream error\n");
            return NULL;
        }

        ret = save_stream(fd, &stream);
        if (ret)
        {
            fclose(fd);
            return NULL;
        }

        IMP_Encoder_ReleaseStream(chnNum, &stream);
    }
    fclose(fd);

    ret = IMP_Encoder_StopRecvPic(chnNum);
    if (ret)
    {
        printf("IMP encode stop recvpic\n");
        return NULL;
    }

    return NULL;
}

int t31_start_stream()
{
    int i = 0;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (!g_video_context[i]->hnd.bRun)
        {
            g_video_context[i]->hnd.bRun = true;
            pthread_create(&g_video_context[i]->hnd.pthread, NULL, _s, (void *)g_video_context[i]);
        }
    }

    return 0;
}

void t31_stop_stream()
{
    int i;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if(g_video_context[i]->hnd.bRun)
        {
            g_video_context[i]->hnd.bRun = false;
            pthread_join(g_video_context[i]->hnd.pthread, NULL);
        }
    }
}

int t31_jpeg_init()
{
    int i, chnNum, ret = 0;
    IMPEncoderChnAttr chnAttr;
    IMPFSChnAttr *imp_chnAttr;

    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            imp_chnAttr = &chn_conf[i].fs_chn_attr;
            chnNum = chn_conf[i].index;
            memset(&chnAttr, 0, sizeof(IMPEncoderChnAttr));
            ret = IMP_Encoder_SetDefaultParam(&chnAttr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
                                              imp_chnAttr->picWidth, imp_chnAttr->picHeight,
                                              imp_chnAttr->outFrmRateNum, imp_chnAttr->outFrmRateDen,
                                              0, 0, 25, 0);

            ret = IMP_Encoder_CreateChn(chn_conf[i].index, &chnAttr);
            if (ret)
            {
                printf("IMP encode create channel JPEG fail\n");
                return -1;
            }

            ret = IMP_Encoder_RegisterChn(chnNum, chn_conf[i].index);
            if (ret)
            {
                printf("IMP encode register channel JPEG fail\n");
                return -1;
            }
        }
    }

    return 0;
}

int t31_jpeg_exit()
{
    int i, ret = 0, chnNum = 0;
    IMPEncoderChnStat chnStat;

    memset(&chnStat, 0, sizeof(IMPEncoderChnStat));
    for (i = 0; i < MAX_STREAM_NUM; i++)
    {
        if (chn_conf[i].enable)
        {
            chnNum = chn_conf[i].index;
            ret = IMP_Encoder_Query(chnNum, &chnStat);
            if (ret)
            {
                printf("IMP encode query channel JPEG fail\n");
                return -1;
            }
            if (chnStat.registered)
            {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret)
                {
                    printf("IMP encoder unRegister channel JPEG\n");
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret)
                {
                    printf("IMP encoder destroy channel JPEG\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}
