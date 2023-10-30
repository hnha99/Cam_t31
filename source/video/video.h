#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <imp_common.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_isp.h>
#include <imp_encoder.h>

#define T31_STREAM_PATH "/tmp"

typedef struct
{
    unsigned int index;
    unsigned int enable;
    IMPEncoderProfile profile;
    IMPFSChnAttr fs_chn_attr;
    IMPCell framesource_chn;
    IMPCell imp_encoder;
} chn_conf_t;

typedef struct
{
    int width;
    int heigth;
} size_pic_t;


#endif // __VIDEO_H__