// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <aac-lib.h>

#include <aacenc_lib.h>
#include <aacdecoder_lib.h>
#include <sstream>

struct AACEncoderOption{
    int sampleRate;
    int channels;

};
static TRANSPORT_TYPE GetTransportType(TransportType transportType){
    switch (transportType)
    {
    case TransportType::RAW:return TT_MP4_RAW;break;
    case TransportType::ADIF:return TT_MP4_ADIF;break;
    case TransportType::ADTS:return TT_MP4_ADTS;break;
    case TransportType::LATM_MPC1:return TT_MP4_LATM_MCP1;break;
    case TransportType::LATM_MPC0:return TT_MP4_LATM_MCP0;break;
    case TransportType::LOAS:return TT_MP4_LOAS;break;
    default:
        throw std::runtime_error("Unknown Transport Type");
    }
}
static AUDIO_OBJECT_TYPE GetAudioObjectType(AudioObjectType aot){
    switch (aot)
    {
    case AudioObjectType::AAC_LC:return AOT_AAC_LC;break;
    case AudioObjectType::AAC_HE:return AOT_SBR;break;
    case AudioObjectType::AAC_HE_V2:return AOT_PS;break;
    case AudioObjectType::AAC_LD:return AOT_ER_AAC_LD;break;
    case AudioObjectType::AAC_ELD:return AOT_ER_AAC_ELD;break;
    default:
        throw std::runtime_error("Unknown AOT");
    }
}
static AudioObjectType FromAudioObjectType(AUDIO_OBJECT_TYPE aot){
    switch (aot)
    {
    case AOT_AAC_LC:return AudioObjectType::AAC_LC;
    case AOT_SBR:return AudioObjectType::AAC_HE;
    case AOT_PS:return AudioObjectType::AAC_HE_V2;
    case AOT_ER_AAC_LD:return AudioObjectType::AAC_LD;
    case AOT_ER_AAC_ELD:return AudioObjectType::AAC_ELD;
    default:
        throw std::runtime_error("Unknown AOT");
    }
}
static CHANNEL_MODE GetChannelMode(int channel){
    switch (channel)
    {
    case 1:return MODE_1;
    case 2:return MODE_2;
    case 3:return MODE_1_2;
    case 4:return MODE_1_2_1;
    case 5:return MODE_1_2_2;
    case 6:return MODE_1_2_2_1;
    case 7:return MODE_1_2_2_2_1;
    default:
        throw std::runtime_error("Unknown channel mode");
    }
}
static int GetChannelOrder(ChannelOrder order){
    switch (order)
    {
    case ChannelOrder::MPEG:return 0;
    case ChannelOrder::WAV:return 1;
    default:
        throw std::runtime_error("Unknown channel order");
    }
}

class AACEncoder:public ICodec{
    HANDLE_AACENCODER mEncoder;
    CodecOption mOption;
    void const*pData=nullptr;
    size_t mLen=0;
    static void CHECK_ERROR(AACENC_ERROR error,char const*name){
    if(error!=AACENC_OK){
        std::stringstream ss;
        ss<<name<<": "<<std::hex<<error;
        throw std::runtime_error(ss.str());
    }
}
public:
    AACEncoder(CodecOption const&option):mEncoder(nullptr),mOption(option){
        auto err=aacEncOpen(&mEncoder,0,0);
        CHECK_ERROR(err,"aacEncOpen");
        TRANSPORT_TYPE transportType=GetTransportType(mOption.transportType);
        AUDIO_OBJECT_TYPE audioObjectType=GetAudioObjectType(mOption.audioObjectType);
        CHANNEL_MODE channelMode=GetChannelMode(mOption.channels);
        int channelOrder=GetChannelOrder(mOption.channelOrder);
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_AOT,audioObjectType);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_AOT");
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_TRANSMUX,transportType);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_TRANSMUX");
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_CHANNELORDER,channelOrder);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_CHANNELORDER");
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_CHANNELMODE,mOption.channels);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_CHANNELMODE");
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_SAMPLERATE,mOption.sampleRate);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_SAMPLERATE");
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_BITRATEMODE,mOption.bitrate<0?-mOption.bitrate:0);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_BITRATEMODE");
        if(mOption.bitrate>0){
            err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_BITRATE,mOption.bitrate);
            CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_BITRATE");
        }
        err=aacEncoder_SetParam(mEncoder,AACENC_PARAM::AACENC_AFTERBURNER,1);
        CHECK_ERROR(err,"aacEncoder_SetParam::AACENC_AFTERBURNER");
        err=aacEncEncode(mEncoder,NULL,NULL,NULL,NULL);
        CHECK_ERROR(err,"aacEncEncode::init");
    }
    virtual ~AACEncoder()override{
        auto err=aacEncClose(&mEncoder);
        CHECK_ERROR(err,"aacEncClose");
    }
    virtual CodecOption const&info()const override{
        return mOption;
    };
    size_t write(void const*data,size_t len)override{
        pData=data;
        mLen=len;
        return len;
    }
    size_t read(void*data,size_t len)override{
        AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
        AACENC_InArgs in_args = { 0 };
        AACENC_OutArgs out_args = { 0 };
        int in_buf_el_size=mOption.sampleBits/8;
        int in_buf_size=mLen;
        int in_buf_dentifier=IN_AUDIO_DATA;
        in_buf.numBufs=1;
        in_buf.bufSizes=&in_buf_size;
        in_buf.bufElSizes=&in_buf_el_size;
        in_buf.bufs=(void**)&pData;
        in_buf.bufferIdentifiers=&in_buf_dentifier;

        in_args.numInSamples=in_buf_size/in_buf_el_size;
        in_args.numAncBytes=0;

        
        int out_buf_el_size=mOption.sampleBits/16;
        int out_buf_size=len/out_buf_el_size;
        int out_buf_dentifier=OUT_BITSTREAM_DATA;
        out_buf.numBufs=1;
        out_buf.bufSizes=&out_buf_size;
        out_buf.bufElSizes=&out_buf_el_size;
        out_buf.bufs=(void**)&data;
        out_buf.bufferIdentifiers=&out_buf_dentifier;

        auto err=aacEncEncode(mEncoder,&in_buf,&out_buf,&in_args,&out_args);
        if(err==AACENC_ENCODE_EOF){
            return 0;
        }
        CHECK_ERROR(err,"aacEncEncode");
        return out_args.numOutBytes;

    }
};


class AACDecoder:public ICodec{
    HANDLE_AACDECODER mDecoder;
    CodecOption mOption;
    static void CHECK_ERROR(AAC_DECODER_ERROR  error,char const*name){
    if(error!=AAC_DEC_OK){
        std::stringstream ss;
        ss<<name<<": "<<std::hex<<error;
        throw std::runtime_error(ss.str());
    }
}
    static uint64_t make_asc2(int aot,int freq,int channels){
        
        uint64_t result=0;
        char*p=(char*)&result;
        char index=0;
        if(aot>=31){
            index+=5;
            for(auto i=0;i<5;++i){
                p[(index-i)/8]|=1<<((8-(index-i)%8));
            }
            index+=6;
            int val=(aot-32)&0x3F;
            for(auto i=0;i<6;++i){
                p[(index-i)/8]|=(!!(val&(1<<i)))<<(8-((index-i)%8));
            }
        }else{
            index+=5;
            for(auto i=0;i<5;++i){
                p[(index-i)/8]|=(!!(aot&(1<<i)))<<(8-((index-i)%8));
            }
        }
        if(freq>=15){
            index+=4;
            for(auto i=0;i<4;++i){
                p[(index-i)/8]|=1<<((8-(index-i)%8));
            }
            index+=24;
            int val=freq&0xFFFFFF;
            for(auto i=0;i<24;++i){
                p[(index-i)/8]|=(!!(val&(1<<i)))<<(8-((index-i)%8));
            }
        }else{
            index+=4;
            for(auto i=0;i<4;++i){
                p[(index-i)/8]|=(!!(freq&(1<<i)))<<(8-((index-i)%8));
            }
        }
        index+=4;
        for(auto i=0;i<4;++i){
                p[(index-i)/8]|=(!!(channels&(1<<i)))<<(8-((index-i)%8));
        }
        return result;
    };
public:
    AACDecoder(CodecOption const&option):mDecoder(NULL),mOption(option){
        TRANSPORT_TYPE tt=GetTransportType(option.transportType);
        mDecoder=aacDecoder_Open(tt,2);
        auto asc=make_asc2(GetAudioObjectType(option.audioObjectType),option.sampleRate,option.channels);
        UCHAR*conf=(UCHAR*)&asc;
        UINT len=sizeof(asc);
        auto err=aacDecoder_ConfigRaw(mDecoder,&conf,&len);
        CHECK_ERROR(err,"aacDecoder_ConfigRaw");

    }
    ~AACDecoder(){
        aacDecoder_Close(mDecoder);
    }
    CodecOption const&info()const override{
        return mOption;
    };
    size_t write(void const*data,size_t len)override{
        UCHAR*buffer=(UCHAR*)data;
        UINT bufferSize=len;
        UINT validBytes=len;
        auto err=aacDecoder_Fill(mDecoder,(UCHAR**)&data,&bufferSize,&validBytes);
        CHECK_ERROR(err,"aacDecoder_Fill");
        return bufferSize-validBytes;
    }
    
    size_t read(void*data,size_t len)override{
        INT_PCM*pcm=(INT_PCM*)data;
        INT pcmLen=len/sizeof(INT_PCM);
        auto err=aacDecoder_DecodeFrame(mDecoder,pcm,pcmLen,0);
        if(err==AAC_DEC_NOT_ENOUGH_BITS){
            return 0;
        }
        CHECK_ERROR(err,"aacDecoder_DecodeFrame");
        auto info=aacDecoder_GetStreamInfo(mDecoder);
        mOption.sampleRate=info->sampleRate;
        mOption.sampleBits=sizeof(INT_PCM)*8;
        mOption.channels=info->numChannels;
        mOption.audioObjectType=FromAudioObjectType(info->aot);
        return info->frameSize*info->numChannels*sizeof(INT_PCM);
    }

};


std::unique_ptr<ICodec> createAACEncoder(CodecOption const&option){
    return std::make_unique<AACEncoder>(option);
}
std::unique_ptr<ICodec> createAACDecoder(TransportType transportType){
    return std::make_unique<AACDecoder>(transportType);
}