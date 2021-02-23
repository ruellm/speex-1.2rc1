#pragma once

#if defined(spexproj_EXPORTS) || defined(EXPORT_DLL)
extern "C"
{
#else
namespace spexproj
{
#endif

    struct soundbuf;

    void* create(int qlty);
    void destroy(void* inst);

    int getCompressionType();
    int getSamplingRate(int quality);
    int getFrameSize(void* inst, int quality);
    // Inplace encoding of a buffer... return new buffer length...
    int compress(void* inst, int len, char* buf);

    // Inplace decoding of a buffer...  return resultant uncompressed length
    int decompress(void* inst, int len, char* buf);

    int decompress_chan(void* inst, int len, char* buf, int uid);

    // network stuff
    void RtpToSb(soundbuf* sb, int paylen, unsigned char* payload);
    int SbToRtp(soundbuf* sb, soundbuf* rp);
}
