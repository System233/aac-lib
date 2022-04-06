# aac-lib

A simple c++ library for fdk-aac.


## API
```cpp
class ICodec{
public:
    virtual CodecOption const&info()const=0;
    virtual size_t write(void const*data,size_t len)=0;
    virtual size_t read(void*data,size_t len)=0;
    virtual ~ICodec(){};
};
std::unique_ptr<ICodec> createAACEncoder(CodecOption const&option);
std::unique_ptr<ICodec> createAACDecoder(TransportType transportType);
```

## Usage
```cpp

auto encoder=createAACEncoder();
auto writtenPCMSize=encoder->write(pcmBuffer,pcmBufferSize);
auto encodedAACSize=encoder->read(buffer,bufferSize);

auto decoder=createAACDecoder();
decoder->write(buffer,encodedAACSize);//the buffer must remain valid until read!
auto decodedPCMSize=decoder->read(pcmBuffer,pcmBufferSize);

```