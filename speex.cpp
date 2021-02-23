// speex.cpp : Defines the entry point for the DLL application.
//
#include "speexproj.h"
#include <speex/speex.h>
#ifdef _WIN32
#include <cstdint>
#include <windows.h>
#include <windowsx.h>
#endif
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef FIND_MEMORY_LEAKS
#include "vld.h" // leak detector
#endif

#ifdef _WIN32
#pragma warning(once : 4244)
#endif

#define BUFL 10200

#define fCompSPEEX 64

#if defined(spexproj_EXPORTS) || defined(EXPORT_DLL)
extern "C"
{
#else
namespace spexproj
{
#endif

    struct soundbuf
    {
        int32_t compression;
        char sendinghost[16];
        struct
        {
            int32_t buffer_len;
            char buffer_val[BUFL];
        } buffer;
    };
    typedef struct soundbuf soundbuf;

#ifdef _WIN32
    void revshort(short FAR* s);
#else
void revshort(short* s);
#endif

    short toShort(float f);
    bool SanityCheck(char* payload);

    struct SpeexInstanceStruct
    {
        // Global handle to the speex encoder.
        void* state;
        SpeexBits bits;
        int fs;
        float* inf;

        // Global handles to the speex structures for decoding.
        void* state_d[10];
        time_t LastUsed[10];
        int UidsPerState[10];
        SpeexBits bits_d;
        float* inf_d;
    };

    typedef struct SpeexInstanceStruct SpeexInstance;

    void* pickDecoderState(SpeexInstance* instance, int uid);

#ifdef _WIN32
#define bcopy(s, d, l) _fmemcpy(d, s, l)
#endif

    bool SanityCheck(char* payload)
    {
        char* ptr = payload;
        short nextLen;

        nextLen = (short)*ptr;
        ptr += sizeof(short); /* increment to next position */

        int packetCnt = 0;

        while (nextLen && packetCnt <= 12)
        { /* No more than 12 packets */

            if ((ptr + nextLen - payload) > BUFL)
                return false;
            else
                ptr += nextLen;

            if (((short)*ptr) && nextLen != (short)*ptr) /*  All frames must be same size
                                                          */
                return false;
            else
            {
                nextLen = (short)*ptr;
                ptr += sizeof(short);
            }

            packetCnt++;
        }

        if (!nextLen)
            return true;
        else
            return false;
    }

    void RtpToSb(soundbuf* sb, int paylen, unsigned char* payload)
    {
        bcopy(payload, sb->buffer.buffer_val, paylen);
        sb->buffer.buffer_len = paylen;

        sb->compression |= fCompSPEEX;
    }

    int SbToRtp(soundbuf* sb, soundbuf* rp)
    {

        bcopy(sb->buffer.buffer_val, ((char*)rp) + 12, (int)sb->buffer.buffer_len);

        return sb->buffer.buffer_len + 12;
    }

    int getCompressionType()
    {
        return fCompSPEEX;
    }

    void* create(int quality)
    {
        if ((quality < 1) || (quality > 3))
        {
            return 0;
        }

        int tmp = 5;
        int tmpC = 4;
        int q = 0;
        int x = 0;
        int y = 0;
        int vbr = 1;
        float fquality = 8.0;
        int vad = 1;
        int dtx = 1;

        SpeexInstance* instance = (SpeexInstance*)calloc(1, sizeof(SpeexInstance));

        // Initialize speex instance.
        for (int i = 0; i < 10; ++i)
        {
            instance->state_d[i] = nullptr;
            instance->LastUsed[i] = 0;
            instance->UidsPerState[i] = 0;
        }
        instance->inf_d = nullptr;
        instance->state = nullptr;
        instance->inf = nullptr;

        for (x = 0; x < sizeof(instance->LastUsed) / sizeof(time_t); ++x)
        {
            instance->LastUsed[x] = 0;
        }
        for (y = 0; y < sizeof(instance->UidsPerState) / sizeof(int); ++y)
        {
            instance->UidsPerState[y] = 0;
        }
        switch (quality)
        {
            case 1:
                instance->state = speex_encoder_init(&speex_nb_mode);
                for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
                {
                    instance->state_d[q] = speex_decoder_init(&speex_nb_mode);
                }
                // state_d[1] = speex_decoder_init ( &speex_nb_mode );
                tmp = 9;
                speex_encoder_ctl(instance->state, SPEEX_SET_QUALITY, &tmp);
                break;
            case 2:
                instance->state = speex_encoder_init(&speex_wb_mode);
                for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
                {
                    instance->state_d[q] = speex_decoder_init(&speex_wb_mode);
                }
                tmp = 6;
                // fquality = 4.0;
                speex_encoder_ctl(instance->state, SPEEX_SET_QUALITY, &tmp);
                // speex_encoder_ctl(state, SPEEX_SET_VBR, &vbr );
                // speex_encoder_ctl(state, SPEEX_SET_VBR_QUALITY, &quality );
                // speex_encoder_ctl(state, SPEEX_SET_VAD, &vad );
                // speex_encoder_ctl(state, SPEEX_SET_DTX, &dtx );
                break;
            // Case 3 is variable bitrate... used for super IM...
            case 3:
                instance->state = speex_encoder_init(&speex_wb_mode);
                for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
                {
                    instance->state_d[q] = speex_decoder_init(&speex_wb_mode);
                }
                tmp = 8;
                // fquality=8.0;
                speex_encoder_ctl(instance->state, SPEEX_SET_QUALITY, &tmp);
                // speex_encoder_ctl(state, SPEEX_SET_VBR, &vbr );
                // speex_encoder_ctl(state, SPEEX_SET_VBR_QUALITY, &quality );
                // speex_encoder_ctl(state, SPEEX_SET_VAD, &vad );
                // speex_encoder_ctl(state, SPEEX_SET_DTX, &dtx );
                break;
        }
        speex_bits_init(&instance->bits);
        speex_bits_init(&instance->bits_d);

        /*  Obtain the derived speex frame size */
        speex_encoder_ctl(instance->state, SPEEX_GET_FRAME_SIZE, &instance->fs);

        /*Set the perceptual enhancement on*/
        int tmp2 = 1;
        for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
        {
            speex_decoder_ctl(instance->state_d[q], SPEEX_SET_ENH, &tmp2);
        }

        // Allocate some space for floating points.
        instance->inf = (float*)calloc(1, sizeof(float) * instance->fs);
        instance->inf_d = (float*)calloc(1, sizeof(float) * instance->fs);

        return (void*)instance;
    }

    int getSamplingRate(int quality)
    {
        int ret = 0;
        switch (quality)
        {
            case 1: ret = 8000; break;
            case 2: ret = 16000; break;
            case 3: ret = 16000; break;
        }

        return ret;
    }

    int getFrameSize(void* inst, int quality)
    {
        SpeexInstance* instance = (SpeexInstance*)inst;

        return instance->fs;
    }

    // Inplace encoding of a buffer... return new buffer length...
    int compress(void* inst, int len, char* buf)
    {
        int i, j, l = 0;
        char* dp = buf;
        short* in = (short*)buf;
        long ldata = len;

        char tempDst[2000];

        SpeexInstance* instance = (SpeexInstance*)inst;

        for (i = 0; i < ldata; i += instance->fs)
        {

            memset(instance->inf, 0, sizeof(float) * instance->fs);

            for (j = 0; j < instance->fs; j++)
            {
                instance->inf[j] = (float)in[i + j];
            }

            speex_bits_reset(&instance->bits);
            speex_encode(instance->state, instance->inf, &instance->bits);
            short nbBytes = speex_bits_write(&instance->bits, tempDst, 2000);

            *((short*)dp) = nbBytes;
            dp += sizeof(short);
            l += sizeof(short);

            memcpy(dp, tempDst, nbBytes);

            dp += nbBytes;
            l += nbBytes;
        }
        // Demarc the end of data with zero size...
        *((short*)dp) = 0;
        l += sizeof(short);

        // Not hiding uncompressed length anymore...
        /* Hide original uncompressed buffer length in first 2 bytes of buffer. */
        /*   *((short *) buf) = (short) ldata;
           revshort((short *) buf); */

        // return compressed length...
        return (l);
    }

    // Inplace decoding of a buffer...  return resultant uncompressed length
    int decompress_chan(void* inst, int len, char* buf, int uid)
    {
        int i, j, l = 0;
        char* dpx = buf;
        char dcb[BUFL];

        SpeexInstance* instance = (SpeexInstance*)inst;

        if (!SanityCheck(buf))
        {
            // memset ( buf, 0, BUFL ); // Perhaps do nothing with the packet...
            return 0;
        }

        memset(dcb, 0, BUFL);

        if (len > 3000)
            return 0;

        //    if (declen <= 0 || declen > 1600) {
        //        declen = 1600;
        //    }

        void* st = pickDecoderState(instance, uid);

        short sLen = *((short*)dpx);
        dpx += sizeof(short);
        j = 0;
        while (sLen > 0)
        {

            if (sLen >= (len - (dpx - buf))) // Additional sanity check to prevent buffer
                                             // overruns.
                break;

            speex_bits_reset(&instance->bits_d);
            speex_bits_read_from(&instance->bits_d, dpx, sLen);
            int y = speex_decode(st, &instance->bits_d, instance->inf_d);

            for (i = 0; i < instance->fs; i++)
            {

                ((short*)dcb)[i + instance->fs * j] = toShort(instance->inf_d[i]);
            }

            dpx += sLen;

            sLen = *((short*)dpx);

            dpx += sizeof(short);
            ++j;
        }
        bcopy(dcb, buf, instance->fs * j * 2);
        return instance->fs * j;
    }

    int decompress(void* inst, int len, char* buf)
    {
        return decompress_chan(inst, len, buf, -1);
    }

#ifdef _WIN32
    void revshort(short FAR* s)
    {
        short s1 = *s;
        LPSTR ip = (LPSTR)&s1, op = (LPSTR)s;

        op[0] = ip[1];
        op[1] = ip[0];
    }
#else
void revshort(short* s)
{
    short s1 = *s;
    char* ip = (char*)&s1;
    char* op = (char*)s;

    op[0] = ip[1];
    op[1] = ip[0];
}
#endif

    void destroy(void* inst)
    {
        SpeexInstance* instance = (SpeexInstance*)inst;

        speex_encoder_destroy(instance->state);
        int q = 0;
        for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
        {
            speex_decoder_destroy(instance->state_d[q]);
        }

        speex_bits_destroy(&instance->bits);
        speex_bits_destroy(&instance->bits_d);

        free(instance->inf);
        free(instance->inf_d);

        instance->inf = instance->inf_d = (float*)nullptr;
        instance->state = nullptr;
        for (q = 0; q < sizeof(instance->state_d) / sizeof(void*); ++q)
        {
            instance->state_d[q] = nullptr;
        }
    }

    void* pickDecoderState(SpeexInstance* inst, int uid)
    {
        static int UsedCnt = 0;
        int t = 0;
        SpeexInstance* instance = (SpeexInstance*)inst;
        int ArrSize = sizeof(instance->UidsPerState) / sizeof(int);

        if (uid < 0)
            return instance->state_d[0];

        // Kill channels which haven't been used in awhile.
        UsedCnt++;
        if (!(UsedCnt % 10))
        {
            for (t = 0; t < ArrSize; ++t)
            {
                time_t t0;
                time(&t0);
                int ThisDiff = (int)(t0 - instance->LastUsed[t]);
                if (ThisDiff > 2 && instance->UidsPerState[t])
                {
                    int tmp = 1;
                    int rc =
                        speex_decoder_ctl(instance->state_d[t], SPEEX_RESET_STATE, &tmp);
                    instance->UidsPerState[t] = 0;
                    instance->LastUsed[t] = 0;
                }
            }
        }

        // Take any slot which we have already used...
        for (t = 0; t < ArrSize; ++t)
        {
            if (instance->UidsPerState[t] == uid)
            {
                //			char str[64];
                //			sprintf ( str, "Pre-assgn %d uid, %d chan\n", uid, t );
                time(&(instance->LastUsed[t]));
                //			::OutputDebugStr ( str );
                return instance->state_d[t];
            }
        }

        // Next failing finding any matching our UID, pick the first '0' entry.
        for (t = 0; t < ArrSize; ++t)
        {
            if (instance->UidsPerState[t] == 0)
            {
                //			char str[64];
                //			sprintf ( str, "Zero-assgn %d uid, %d chan\n", uid, t );
                time(&(instance->LastUsed[t]));
                instance->UidsPerState[t] = uid;
                //			::OutputDebugStr ( str );
                return instance->state_d[t];
            }
        }

        // Failing the above 2, we replace the oldest channel...  if it's old enough...
        int Eldest = -1;
        int LargestDiff = -1;
        for (t = 0; t < ArrSize; ++t)
        {
            time_t t0;
            time(&t0);
            int ThisDiff = (int)(t0 - instance->LastUsed[t]);
            if (ThisDiff > LargestDiff)
            {
                LargestDiff = ThisDiff;
                Eldest = t;
            }
        }
        // If the oldest age is 1 1/2 seconds or more, then return that channel
        if (LargestDiff > 2)
        {
            //		char str[64];
            //		sprintf ( str, "Old assign %d uid, %d chan\n", uid, Eldest );
            time(&(instance->LastUsed[Eldest]));
            instance->UidsPerState[Eldest] = uid;
            //		::OutputDebugStr ( str );
            return instance->state_d[Eldest];
        }

        // Lastly give up and return the 0th channel.... (SHOULD NEVER HAPPEN!)
        //	::OutputDebugStr ( "Oh stuff\n" );

        return instance->state_d[0];
    }

    short toShort(float f)
    {
        return (f > 32767.0) ? ((short)32767) :
                               ((f < -32767.49) ? ((short)-32768) : (short)f);
    }
}
