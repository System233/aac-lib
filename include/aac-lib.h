// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#ifndef AAC_LIB_H

#define AAC_LIB_H


#include <cstdint>
#include <memory>
enum class ChannelOrder{
    UNKNOWN,
    MPEG=0,
    WAV=1
};
enum class TransportType{
    UNKNOWN,
    RAW,
    ADIF,
    ADTS,
    LATM_MPC1,
    LATM_MPC0,
    LOAS
};
enum class AudioObjectType{
    UNKNOWN,
    AAC_LC,
    AAC_HE,
    AAC_HE_V2,
    AAC_LD,
    AAC_ELD
};
enum class SampleRate:int{
    R8000=8000, 
    R11025=11025,
    R12000=12000,
    R16000=16000, 
    R22050=22050, 
    R24000=24000, 
    R32000=32000, 
    R44100=44100,
    R48000=48000,
    R64000=64000,
    R88200=88200,
    R96000=96000
};
enum class SampleBits:int{
    B8=8,
    B16=16,
    B24=24,
    B32=32,
    B64=64
};
struct CodecOption{
    int sampleRate=0;
    int sampleBits=0;
    int channels=0;
    AudioObjectType audioObjectType=AudioObjectType::UNKNOWN;
    ChannelOrder channelOrder=ChannelOrder::UNKNOWN;
    TransportType transportType=TransportType::UNKNOWN;
    int bitrate=0;

};
class ICodec{
public:
    virtual CodecOption const&info()const=0;
    virtual size_t write(void const*data,size_t len)=0;
    virtual size_t read(void*data,size_t len)=0;
    virtual ~ICodec(){};
};


std::unique_ptr<ICodec> createAACEncoder(CodecOption const&option);
std::unique_ptr<ICodec> createAACDecoder(CodecOption const&option);

#endif //AAC_LIB_H