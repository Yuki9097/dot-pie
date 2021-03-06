/* Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#define LOG_TAG "LocSvc_SystemStatus"

#include <inttypes.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <platform_lib_log_util.h>
#include <MsgTask.h>
#include <loc_nmea.h>
#include <DataItemsFactoryProxy.h>
#include <SystemStatus.h>
#include <SystemStatusOsObserver.h>

namespace loc_core
{

/******************************************************************************
 SystemStatusNmeaBase - base class for all NMEA parsers
******************************************************************************/
class SystemStatusNmeaBase
{
protected:
    std::vector<std::string> mField;

    SystemStatusNmeaBase(const char *str_in, uint32_t len_in)
    {
        // check size and talker
        if (!loc_nmea_is_debug(str_in, len_in)) {
            return;
        }

        std::string parser(str_in);
        std::string::size_type index = 0;

        // verify checksum field
        index = parser.find("*");
        if (index == std::string::npos) {
            return;
        }
        parser[index] = ',';

        // tokenize parser
        while (1) {
            std::string str;
            index = parser.find(",");
            if (index == std::string::npos) {
                break;
            }
            str = parser.substr(0, index);
            parser = parser.substr(index + 1);
            mField.push_back(str);
        }
    }

    virtual ~SystemStatusNmeaBase() { }

public:
    static const uint32_t NMEA_MINSIZE = DEBUG_NMEA_MINSIZE;
    static const uint32_t NMEA_MAXSIZE = DEBUG_NMEA_MAXSIZE;
};

/******************************************************************************
 SystemStatusPQWM1
******************************************************************************/
class SystemStatusPQWM1
{
public:
    uint16_t mGpsWeek;    // x1
    uint32_t mGpsTowMs;   // x2
    uint8_t  mTimeValid;  // x3
    uint8_t  mTimeSource; // x4
    int32_t  mTimeUnc;    // x5
    int32_t  mClockFreqBias; // x6
    int32_t  mClockFreqBiasUnc; // x7
    uint8_t  mXoState;    // x8
    int32_t  mPgaGain;    // x9
    uint32_t mGpsBpAmpI;  // xA
    uint32_t mGpsBpAmpQ;  // xB
    uint32_t mAdcI;       // xC
    uint32_t mAdcQ;       // xD
    uint32_t mJammerGps;  // xE
    uint32_t mJammerGlo;  // xF
    uint32_t mJammerBds;  // x10
    uint32_t mJammerGal;  // x11
    uint32_t mRecErrorRecovery; // x12
    double   mAgcGps;     // x13
    double   mAgcGlo;     // x14
    double   mAgcBds;     // x15
    double   mAgcGal;     // x16
    int32_t  mLeapSeconds;// x17
    int32_t  mLeapSecUnc; // x18
};

// parser
class SystemStatusPQWM1parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eGpsWeek = 1,
        eGpsTowMs = 2,
        eTimeValid = 3,
        eTimeSource = 4,
        eTimeUnc = 5,
        eClockFreqBias = 6,
        eClockFreqBiasUnc = 7,
        eXoState = 8,
        ePgaGain = 9,
        eGpsBpAmpI = 10,
        eGpsBpAmpQ = 11,
        eAdcI = 12,
        eAdcQ = 13,
        eJammerGps = 14,
        eJammerGlo = 15,
        eJammerBds = 16,
        eJammerGal = 17,
        eRecErrorRecovery = 18,
        eAgcGps = 19,
        eAgcGlo = 20,
        eAgcBds = 21,
        eAgcGal = 22,
        eLeapSeconds = 23,
        eLeapSecUnc = 24,
        eMax
    };
    SystemStatusPQWM1 mM1;

public:
    inline uint16_t   getGpsWeek()    { return mM1.mGpsWeek; }
    inline uint32_t   getGpsTowMs()   { return mM1.mGpsTowMs; }
    inline uint8_t    getTimeValid()  { return mM1.mTimeValid; }
    inline uint8_t    getTimeSource() { return mM1.mTimeSource; }
    inline int32_t    getTimeUnc()    { return mM1.mTimeUnc; }
    inline int32_t    getClockFreqBias() { return mM1.mClockFreqBias; }
    inline int32_t    getClockFreqBiasUnc() { return mM1.mClockFreqBiasUnc; }
    inline uint8_t    getXoState()    { return mM1.mXoState;}
    inline int32_t    getPgaGain()    { return mM1.mPgaGain;          }
    inline uint32_t   getGpsBpAmpI()  { return mM1.mGpsBpAmpI;        }
    inline uint32_t   getGpsBpAmpQ()  { return mM1.mGpsBpAmpQ;        }
    inline uint32_t   getAdcI()       { return mM1.mAdcI;             }
    inline uint32_t   getAdcQ()       { return mM1.mAdcQ;             }
    inline uint32_t   getJammerGps()  { return mM1.mJammerGps;        }
    inline uint32_t   getJammerGlo()  { return mM1.mJammerGlo;        }
    inline uint32_t   getJammerBds()  { return mM1.mJammerBds;        }
    inline uint32_t   getJammerGal()  { return mM1.mJammerGal;        }
    inline uint32_t   getAgcGps()     { return mM1.mAgcGps;           }
    inline uint32_t   getAgcGlo()     { return mM1.mAgcGlo;           }
    inline uint32_t   getAgcBds()     { return mM1.mAgcBds;           }
    inline uint32_t   getAgcGal()     { return mM1.mAgcGal;           }
    inline uint32_t   getRecErrorRecovery() { return mM1.mRecErrorRecovery; }
    inline int32_t    getLeapSeconds(){ return mM1.mLeapSeconds; }
    inline int32_t    getLeapSecUnc() { return mM1.mLeapSecUnc; }

    SystemStatusPQWM1parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        memset(&mM1, 0, sizeof(mM1));
        if (mField.size() < eMax) {
            LOC_LOGE("PQWM1parser - invalid size=%zu", mField.size());
            mM1.mTimeValid = 0;
            return;
        }
        mM1.mGpsWeek = atoi(mField[eGpsWeek].c_str());
        mM1.mGpsTowMs = atoi(mField[eGpsTowMs].c_str());
        mM1.mTimeValid = atoi(mField[eTimeValid].c_str());
        mM1.mTimeSource = atoi(mField[eTimeSource].c_str());
        mM1.mTimeUnc = atoi(mField[eTimeUnc].c_str());
        mM1.mClockFreqBias = atoi(mField[eClockFreqBias].c_str());
        mM1.mClockFreqBiasUnc = atoi(mField[eClockFreqBiasUnc].c_str());
        mM1.mXoState = atoi(mField[eXoState].c_str());
        mM1.mPgaGain = atoi(mField[ePgaGain].c_str());
        mM1.mGpsBpAmpI = atoi(mField[eGpsBpAmpI].c_str());
        mM1.mGpsBpAmpQ = atoi(mField[eGpsBpAmpQ].c_str());
        mM1.mAdcI = atoi(mField[eAdcI].c_str());
        mM1.mAdcQ = atoi(mField[eAdcQ].c_str());
        mM1.mJammerGps = atoi(mField[eJammerGps].c_str());
        mM1.mJammerGlo = atoi(mField[eJammerGlo].c_str());
        mM1.mJammerBds = atoi(mField[eJammerBds].c_str());
        mM1.mJammerGal = atoi(mField[eJammerGal].c_str());
        mM1.mRecErrorRecovery = atoi(mField[eRecErrorRecovery].c_str());
        mM1.mAgcGps = atof(mField[eAgcGps].c_str());
        mM1.mAgcGlo = atof(mField[eAgcGlo].c_str());
        mM1.mAgcBds = atof(mField[eAgcBds].c_str());
        mM1.mAgcGal = atof(mField[eAgcGal].c_str());
        mM1.mLeapSeconds = atoi(mField[eLeapSeconds].c_str());
        mM1.mLeapSecUnc = atoi(mField[eLeapSecUnc].c_str());
    }

    inline SystemStatusPQWM1& get() { return mM1;} //getparser
};

/******************************************************************************
 SystemStatusPQWP1
******************************************************************************/
class SystemStatusPQWP1
{
public:
    uint8_t  mEpiValidity; // x4
    float    mEpiLat;    // x5
    float    mEpiLon;    // x6
    float    mEpiAlt;    // x7
    float    mEpiHepe;   // x8
    float    mEpiAltUnc; // x9
    uint8_t  mEpiSrc;    // x10
};

class SystemStatusPQWP1parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eEpiValidity = 2,
        eEpiLat = 3,
        eEpiLon = 4,
        eEpiAlt = 5,
        eEpiHepe = 6,
        eEpiAltUnc = 7,
        eEpiSrc = 8,
        eMax
    };
    SystemStatusPQWP1 mP1;

public:
    inline uint8_t    getEpiValidity() { return mP1.mEpiValidity;      }
    inline float      getEpiLat() { return mP1.mEpiLat;           }
    inline float      getEpiLon() { return mP1.mEpiLon;           }
    inline float      getEpiAlt() { return mP1.mEpiAlt;           }
    inline float      getEpiHepe() { return mP1.mEpiHepe;          }
    inline float      getEpiAltUnc() { return mP1.mEpiAltUnc;        }
    inline uint8_t    getEpiSrc() { return mP1.mEpiSrc;           }

    SystemStatusPQWP1parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP1, 0, sizeof(mP1));
        mP1.mEpiValidity = strtol(mField[eEpiValidity].c_str(), NULL, 16);
        mP1.mEpiLat = atof(mField[eEpiLat].c_str());
        mP1.mEpiLon = atof(mField[eEpiLon].c_str());
        mP1.mEpiAlt = atof(mField[eEpiAlt].c_str());
        mP1.mEpiHepe = atoi(mField[eEpiHepe].c_str());
        mP1.mEpiAltUnc = atof(mField[eEpiAltUnc].c_str());
        mP1.mEpiSrc = atoi(mField[eEpiSrc].c_str());
    }

    inline SystemStatusPQWP1& get() { return mP1;}
};

/******************************************************************************
 SystemStatusPQWP2
******************************************************************************/
class SystemStatusPQWP2
{
public:
    float    mBestLat;   // x4
    float    mBestLon;   // x5
    float    mBestAlt;   // x6
    float    mBestHepe;  // x7
    float    mBestAltUnc; // x8
};

class SystemStatusPQWP2parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eBestLat = 2,
        eBestLon = 3,
        eBestAlt = 4,
        eBestHepe = 5,
        eBestAltUnc = 6,
        eMax
    };
    SystemStatusPQWP2 mP2;

public:
    inline float      getBestLat() { return mP2.mBestLat;          }
    inline float      getBestLon() { return mP2.mBestLon;          }
    inline float      getBestAlt() { return mP2.mBestAlt;          }
    inline float      getBestHepe() { return mP2.mBestHepe;         }
    inline float      getBestAltUnc() { return mP2.mBestAltUnc;       }

    SystemStatusPQWP2parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP2, 0, sizeof(mP2));
        mP2.mBestLat = atof(mField[eBestLat].c_str());
        mP2.mBestLon = atof(mField[eBestLon].c_str());
        mP2.mBestAlt = atof(mField[eBestAlt].c_str());
        mP2.mBestHepe = atof(mField[eBestHepe].c_str());
        mP2.mBestAltUnc = atof(mField[eBestAltUnc].c_str());
    }

    inline SystemStatusPQWP2& get() { return mP2;}
};

/******************************************************************************
 SystemStatusPQWP3
******************************************************************************/
class SystemStatusPQWP3
{
public:
    uint8_t   mXtraValidMask;
    uint32_t  mGpsXtraAge;
    uint32_t  mGloXtraAge;
    uint32_t  mBdsXtraAge;
    uint32_t  mGalXtraAge;
    uint32_t  mQzssXtraAge;
    uint32_t  mGpsXtraValid;
    uint32_t  mGloXtraValid;
    uint64_t  mBdsXtraValid;
    uint64_t  mGalXtraValid;
    uint8_t   mQzssXtraValid;
};

class SystemStatusPQWP3parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eXtraValidMask = 2,
        eGpsXtraAge = 3,
        eGloXtraAge = 4,
        eBdsXtraAge = 5,
        eGalXtraAge = 6,
        eQzssXtraAge = 7,
        eGpsXtraValid = 8,
        eGloXtraValid = 9,
        eBdsXtraValid = 10,
        eGalXtraValid = 11,
        eQzssXtraValid = 12,
        eMax
    };
    SystemStatusPQWP3 mP3;

public:
    inline uint8_t    getXtraValid() { return mP3.mXtraValidMask;   }
    inline uint32_t   getGpsXtraAge() { return mP3.mGpsXtraAge;       }
    inline uint32_t   getGloXtraAge() { return mP3.mGloXtraAge;       }
    inline uint32_t   getBdsXtraAge() { return mP3.mBdsXtraAge;       }
    inline uint32_t   getGalXtraAge() { return mP3.mGalXtraAge;       }
    inline uint32_t   getQzssXtraAge() { return mP3.mQzssXtraAge;      }
    inline uint32_t   getGpsXtraValid() { return mP3.mGpsXtraValid;     }
    inline uint32_t   getGloXtraValid() { return mP3.mGloXtraValid;     }
    inline uint64_t   getBdsXtraValid() { return mP3.mBdsXtraValid;     }
    inline uint64_t   getGalXtraValid() { return mP3.mGalXtraValid;     }
    inline uint8_t    getQzssXtraValid() { return mP3.mQzssXtraValid;    }

    SystemStatusPQWP3parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP3, 0, sizeof(mP3));
        mP3.mXtraValidMask = strtol(mField[eXtraValidMask].c_str(), NULL, 16);
        mP3.mGpsXtraAge = atoi(mField[eGpsXtraAge].c_str());
        mP3.mGloXtraAge = atoi(mField[eGloXtraAge].c_str());
        mP3.mBdsXtraAge = atoi(mField[eBdsXtraAge].c_str());
        mP3.mGalXtraAge = atoi(mField[eGalXtraAge].c_str());
        mP3.mQzssXtraAge = atoi(mField[eQzssXtraAge].c_str());
        mP3.mGpsXtraValid = strtol(mField[eGpsXtraValid].c_str(), NULL, 16);
        mP3.mGloXtraValid = strtol(mField[eGloXtraValid].c_str(), NULL, 16);
        mP3.mBdsXtraValid = strtol(mField[eBdsXtraValid].c_str(), NULL, 16);
        mP3.mGalXtraValid = strtol(mField[eGalXtraValid].c_str(), NULL, 16);
        mP3.mQzssXtraValid = strtol(mField[eQzssXtraValid].c_str(), NULL, 16);
    }

    inline SystemStatusPQWP3& get() { return mP3;}
};

/******************************************************************************
 SystemStatusPQWP4
******************************************************************************/
class SystemStatusPQWP4
{
public:
    uint32_t  mGpsEpheValid;
    uint32_t  mGloEpheValid;
    uint64_t  mBdsEpheValid;
    uint64_t  mGalEpheValid;
    uint8_t   mQzssEpheValid;
};

class SystemStatusPQWP4parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eGpsEpheValid = 2,
        eGloEpheValid = 3,
        eBdsEpheValid = 4,
        eGalEpheValid = 5,
        eQzssEpheValid = 6,
        eMax
    };
    SystemStatusPQWP4 mP4;

public:
    inline uint32_t   getGpsEpheValid() { return mP4.mGpsEpheValid;     }
    inline uint32_t   getGloEpheValid() { return mP4.mGloEpheValid;     }
    inline uint64_t   getBdsEpheValid() { return mP4.mBdsEpheValid;     }
    inline uint64_t   getGalEpheValid() { return mP4.mGalEpheValid;     }
    inline uint8_t    getQzssEpheValid() { return mP4.mQzssEpheValid;    }

    SystemStatusPQWP4parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP4, 0, sizeof(mP4));
        mP4.mGpsEpheValid = strtol(mField[eGpsEpheValid].c_str(), NULL, 16);
        mP4.mGloEpheValid = strtol(mField[eGloEpheValid].c_str(), NULL, 16);
        mP4.mBdsEpheValid = strtol(mField[eBdsEpheValid].c_str(), NULL, 16);
        mP4.mGalEpheValid = strtol(mField[eGalEpheValid].c_str(), NULL, 16);
        mP4.mQzssEpheValid = strtol(mField[eQzssEpheValid].c_str(), NULL, 16);
    }

    inline SystemStatusPQWP4& get() { return mP4;}
};

/******************************************************************************
 SystemStatusPQWP5
******************************************************************************/
class SystemStatusPQWP5
{
public:
    uint32_t  mGpsUnknownMask;
    uint32_t  mGloUnknownMask;
    uint64_t  mBdsUnknownMask;
    uint64_t  mGalUnknownMask;
    uint8_t   mQzssUnknownMask;
    uint32_t  mGpsGoodMask;
    uint32_t  mGloGoodMask;
    uint64_t  mBdsGoodMask;
    uint64_t  mGalGoodMask;
    uint8_t   mQzssGoodMask;
    uint32_t  mGpsBadMask;
    uint32_t  mGloBadMask;
    uint64_t  mBdsBadMask;
    uint64_t  mGalBadMask;
    uint8_t   mQzssBadMask;
};

class SystemStatusPQWP5parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eGpsUnknownMask = 2,
        eGloUnknownMask = 3,
        eBdsUnknownMask = 4,
        eGalUnknownMask = 5,
        eQzssUnknownMask = 6,
        eGpsGoodMask = 7,
        eGloGoodMask = 8,
        eBdsGoodMask = 9,
        eGalGoodMask = 10,
        eQzssGoodMask = 11,
        eGpsBadMask = 12,
        eGloBadMask = 13,
        eBdsBadMask = 14,
        eGalBadMask = 15,
        eQzssBadMask = 16,
        eMax
    };
    SystemStatusPQWP5 mP5;

public:
    inline uint32_t   getGpsUnknownMask() { return mP5.mGpsUnknownMask;   }
    inline uint32_t   getGloUnknownMask() { return mP5.mGloUnknownMask;   }
    inline uint64_t   getBdsUnknownMask() { return mP5.mBdsUnknownMask;   }
    inline uint64_t   getGalUnknownMask() { return mP5.mGalUnknownMask;   }
    inline uint8_t    getQzssUnknownMask() { return mP5.mQzssUnknownMask;  }
    inline uint32_t   getGpsGoodMask() { return mP5.mGpsGoodMask;      }
    inline uint32_t   getGloGoodMask() { return mP5.mGloGoodMask;      }
    inline uint64_t   getBdsGoodMask() { return mP5.mBdsGoodMask;      }
    inline uint64_t   getGalGoodMask() { return mP5.mGalGoodMask;      }
    inline uint8_t    getQzssGoodMask() { return mP5.mQzssGoodMask;     }
    inline uint32_t   getGpsBadMask() { return mP5.mGpsBadMask;       }
    inline uint32_t   getGloBadMask() { return mP5.mGloBadMask;       }
    inline uint64_t   getBdsBadMask() { return mP5.mBdsBadMask;       }
    inline uint64_t   getGalBadMask() { return mP5.mGalBadMask;       }
    inline uint8_t    getQzssBadMask() { return mP5.mQzssBadMask;      }

    SystemStatusPQWP5parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP5, 0, sizeof(mP5));
        mP5.mGpsUnknownMask = strtol(mField[eGpsUnknownMask].c_str(), NULL, 16);
        mP5.mGloUnknownMask = strtol(mField[eGloUnknownMask].c_str(), NULL, 16);
        mP5.mBdsUnknownMask = strtol(mField[eBdsUnknownMask].c_str(), NULL, 16);
        mP5.mGalUnknownMask = strtol(mField[eGalUnknownMask].c_str(), NULL, 16);
        mP5.mQzssUnknownMask = strtol(mField[eQzssUnknownMask].c_str(), NULL, 16);
        mP5.mGpsGoodMask = strtol(mField[eGpsGoodMask].c_str(), NULL, 16);
        mP5.mGloGoodMask = strtol(mField[eGloGoodMask].c_str(), NULL, 16);
        mP5.mBdsGoodMask = strtol(mField[eBdsGoodMask].c_str(), NULL, 16);
        mP5.mGalGoodMask = strtol(mField[eGalGoodMask].c_str(), NULL, 16);
        mP5.mQzssGoodMask = strtol(mField[eQzssGoodMask].c_str(), NULL, 16);
        mP5.mGpsBadMask = strtol(mField[eGpsBadMask].c_str(), NULL, 16);
        mP5.mGloBadMask = strtol(mField[eGloBadMask].c_str(), NULL, 16);
        mP5.mBdsBadMask = strtol(mField[eBdsBadMask].c_str(), NULL, 16);
        mP5.mGalBadMask = strtol(mField[eGalBadMask].c_str(), NULL, 16);
        mP5.mQzssBadMask = strtol(mField[eQzssBadMask].c_str(), NULL, 16);
    }

    inline SystemStatusPQWP5& get() { return mP5;}
};

/******************************************************************************
 SystemStatusPQWP6parser
******************************************************************************/
class SystemStatusPQWP6
{
public:
    uint32_t  mFixInfoMask;
};

class SystemStatusPQWP6parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eFixInfoMask = 2,
        eMax
    };
    SystemStatusPQWP6 mP6;

public:
    inline uint32_t   getFixInfoMask() { return mP6.mFixInfoMask;      }

    SystemStatusPQWP6parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mP6, 0, sizeof(mP6));
        mP6.mFixInfoMask = strtol(mField[eFixInfoMask].c_str(), NULL, 16);
    }

    inline SystemStatusPQWP6& get() { return mP6;}
};

/******************************************************************************
 SystemStatusPQWP7parser
******************************************************************************/
class SystemStatusPQWP7
{
public:
    SystemStatusNav mNav[SV_ALL_NUM];
};

class SystemStatusPQWP7parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eMax = 2 + SV_ALL_NUM*3
    };
    SystemStatusPQWP7 mP7;

public:
    SystemStatusPQWP7parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            LOC_LOGE("PQWP7parser - invalid size=%zu", mField.size());
            return;
        }
        for (uint32_t i=0; i<SV_ALL_NUM; i++) {
            mP7.mNav[i].mType   = GnssEphemerisType(atoi(mField[i*3+2].c_str()));
            mP7.mNav[i].mSource = GnssEphemerisSource(atoi(mField[i*3+3].c_str()));
            mP7.mNav[i].mAgeSec = atoi(mField[i*3+4].c_str());
        }
    }

    inline SystemStatusPQWP7& get() { return mP7;}
};

/******************************************************************************
 SystemStatusPQWS1parser
******************************************************************************/
class SystemStatusPQWS1
{
public:
    uint32_t  mFixInfoMask;
    uint32_t  mHepeLimit;
};

class SystemStatusPQWS1parser : public SystemStatusNmeaBase
{
private:
    enum
    {
        eTalker = 0,
        eUtcTime = 1,
        eFixInfoMask = 2,
        eHepeLimit = 3,
        eMax
    };
    SystemStatusPQWS1 mS1;

public:
    inline uint16_t   getFixInfoMask() { return mS1.mFixInfoMask;      }
    inline uint32_t   getHepeLimit()   { return mS1.mHepeLimit;      }

    SystemStatusPQWS1parser(const char *str_in, uint32_t len_in)
        : SystemStatusNmeaBase(str_in, len_in)
    {
        if (mField.size() < eMax) {
            return;
        }
        memset(&mS1, 0, sizeof(mS1));
        mS1.mFixInfoMask = atoi(mField[eFixInfoMask].c_str());
        mS1.mHepeLimit = atoi(mField[eHepeLimit].c_str());
    }

    inline SystemStatusPQWS1& get() { return mS1;}
};

/******************************************************************************
 SystemStatusTimeAndClock
******************************************************************************/
SystemStatusTimeAndClock::SystemStatusTimeAndClock(const SystemStatusPQWM1& nmea) :
    mGpsWeek(nmea.mGpsWeek),
    mGpsTowMs(nmea.mGpsTowMs),
    mTimeValid(nmea.mTimeValid),
    mTimeSource(nmea.mTimeSource),
    mTimeUnc(nmea.mTimeUnc),
    mClockFreqBias(nmea.mClockFreqBias),
    mClockFreqBiasUnc(nmea.mClockFreqBiasUnc),
    mLeapSeconds(nmea.mLeapSeconds),
    mLeapSecUnc(nmea.mLeapSecUnc)
{
}

bool SystemStatusTimeAndClock::equals(SystemStatusTimeAndClock& peer)
{
    if ((mGpsWeek != peer.mGpsWeek) ||
        (mGpsTowMs != peer.mGpsTowMs) ||
        (mTimeValid != peer.mTimeValid) ||
        (mTimeSource != peer.mTimeSource) ||
        (mTimeUnc != peer.mTimeUnc) ||
        (mClockFreqBias != peer.mClockFreqBias) ||
        (mClockFreqBiasUnc != peer.mClockFreqBiasUnc) ||
        (mLeapSeconds != peer.mLeapSeconds) ||
        (mLeapSecUnc != peer.mLeapSecUnc)) {
        return false;
    }
    return true;
}

void SystemStatusTimeAndClock::dump()
{
    LOC_LOGV("TimeAndClock: u=%ld:%ld g=%d:%d v=%d ts=%d tu=%d b=%d bu=%d ls=%d lu=%d",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mGpsWeek,
             mGpsTowMs,
             mTimeValid,
             mTimeSource,
             mTimeUnc,
             mClockFreqBias,
             mClockFreqBiasUnc,
             mLeapSeconds,
             mLeapSecUnc);
    return;
}

/******************************************************************************
 SystemStatusXoState
******************************************************************************/
SystemStatusXoState::SystemStatusXoState(const SystemStatusPQWM1& nmea) :
    mXoState(nmea.mXoState)
{
}

bool SystemStatusXoState::equals(SystemStatusXoState& peer)
{
    if (mXoState != peer.mXoState) {
        return false;
    }
    return true;
}

void SystemStatusXoState::dump()
{
    LOC_LOGV("XoState: u=%ld:%ld x=%d",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mXoState);
    return;
}

/******************************************************************************
 SystemStatusRfAndParams
******************************************************************************/
SystemStatusRfAndParams::SystemStatusRfAndParams(const SystemStatusPQWM1& nmea) :
    mPgaGain(nmea.mPgaGain),
    mGpsBpAmpI(nmea.mGpsBpAmpI),
    mGpsBpAmpQ(nmea.mGpsBpAmpQ),
    mAdcI(nmea.mAdcI),
    mAdcQ(nmea.mAdcQ),
    mJammerGps(nmea.mJammerGps),
    mJammerGlo(nmea.mJammerGlo),
    mJammerBds(nmea.mJammerBds),
    mJammerGal(nmea.mJammerGal),
    mAgcGps(nmea.mAgcGps),
    mAgcGlo(nmea.mAgcGlo),
    mAgcBds(nmea.mAgcBds),
    mAgcGal(nmea.mAgcGal)
{
}

bool SystemStatusRfAndParams::equals(SystemStatusRfAndParams& peer)
{
    if ((mPgaGain != peer.mPgaGain) ||
        (mGpsBpAmpI != peer.mGpsBpAmpI) ||
        (mGpsBpAmpQ != peer.mGpsBpAmpQ) ||
        (mAdcI != peer.mAdcI) ||
        (mAdcQ != peer.mAdcQ) ||
        (mJammerGps != peer.mJammerGps) ||
        (mJammerGlo != peer.mJammerGlo) ||
        (mJammerBds != peer.mJammerBds) ||
        (mJammerGal != peer.mJammerGal) ||
        (mAgcGps != peer.mAgcGps) ||
        (mAgcGlo != peer.mAgcGlo) ||
        (mAgcBds != peer.mAgcBds) ||
        (mAgcGal != peer.mAgcGal)) {
        return false;
    }
    return true;
}

void SystemStatusRfAndParams::dump()
{
    LOC_LOGV("RfAndParams: u=%ld:%ld p=%d bi=%d bq=%d ai=%d aq=%d "
             "jgp=%d jgl=%d jbd=%d jga=%d "
             "agp=%lf agl=%lf abd=%lf aga=%lf",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mPgaGain,
             mGpsBpAmpI,
             mGpsBpAmpQ,
             mAdcI,
             mAdcQ,
             mJammerGps,
             mJammerGlo,
             mJammerBds,
             mJammerGal,
             mAgcGps,
             mAgcGlo,
             mAgcBds,
             mAgcGal);
    return;
}

/******************************************************************************
 SystemStatusErrRecovery
******************************************************************************/
SystemStatusErrRecovery::SystemStatusErrRecovery(const SystemStatusPQWM1& nmea) :
    mRecErrorRecovery(nmea.mRecErrorRecovery)
{
}

bool SystemStatusErrRecovery::equals(SystemStatusErrRecovery& peer)
{
    if (mRecErrorRecovery != peer.mRecErrorRecovery) {
        return false;
    }
    return true;
}

void SystemStatusErrRecovery::dump()
{
    LOC_LOGV("ErrRecovery: u=%ld:%ld e=%d",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mRecErrorRecovery);
    return;
}

/******************************************************************************
 SystemStatusInjectedPosition
******************************************************************************/
SystemStatusInjectedPosition::SystemStatusInjectedPosition(const SystemStatusPQWP1& nmea) :
    mEpiValidity(nmea.mEpiValidity),
    mEpiLat(nmea.mEpiLat),
    mEpiLon(nmea.mEpiLon),
    mEpiAlt(nmea.mEpiAlt),
    mEpiHepe(nmea.mEpiHepe),
    mEpiAltUnc(nmea.mEpiAltUnc),
    mEpiSrc(nmea.mEpiSrc)
{
}

bool SystemStatusInjectedPosition::equals(SystemStatusInjectedPosition& peer)
{
    if ((mEpiValidity != peer.mEpiValidity) ||
        (mEpiLat != peer.mEpiLat) ||
        (mEpiLon != peer.mEpiLon) ||
        (mEpiAlt != peer.mEpiAlt) ||
        (mEpiHepe != peer.mEpiHepe) ||
        (mEpiAltUnc != peer.mEpiAltUnc) ||
        (mEpiSrc != peer.mEpiSrc)) {
        return false;
    }
    return true;
}

void SystemStatusInjectedPosition::dump()
{
    LOC_LOGV("InjectedPosition: u=%ld:%ld v=%x la=%f lo=%f al=%f he=%f au=%f es=%d",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mEpiValidity,
             mEpiLat,
             mEpiLon,
             mEpiAlt,
             mEpiHepe,
             mEpiAltUnc,
             mEpiSrc);
    return;
}

/******************************************************************************
 SystemStatusBestPosition
******************************************************************************/
SystemStatusBestPosition::SystemStatusBestPosition(const SystemStatusPQWP2& nmea) :
    mValid(true),
    mBestLat(nmea.mBestLat),
    mBestLon(nmea.mBestLon),
    mBestAlt(nmea.mBestAlt),
    mBestHepe(nmea.mBestHepe),
    mBestAltUnc(nmea.mBestAltUnc)
{
}

bool SystemStatusBestPosition::equals(SystemStatusBestPosition& peer)
{
    if ((mBestLat != peer.mBestLat) ||
        (mBestLon != peer.mBestLon) ||
        (mBestAlt != peer.mBestAlt) ||
        (mBestHepe != peer.mBestHepe) ||
        (mBestAltUnc != peer.mBestAltUnc)) {
        return false;
    }
    return true;
}

void SystemStatusBestPosition::dump()
{
    LOC_LOGV("BestPosition: u=%ld:%ld la=%f lo=%f al=%f he=%f au=%f",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mBestLat,
             mBestLon,
             mBestAlt,
             mBestHepe,
             mBestAltUnc);
    return;
}

/******************************************************************************
 SystemStatusXtra
******************************************************************************/
SystemStatusXtra::SystemStatusXtra(const SystemStatusPQWP3& nmea) :
    mXtraValidMask(nmea.mXtraValidMask),
    mGpsXtraAge(nmea.mGpsXtraAge),
    mGloXtraAge(nmea.mGloXtraAge),
    mBdsXtraAge(nmea.mBdsXtraAge),
    mGalXtraAge(nmea.mGalXtraAge),
    mQzssXtraAge(nmea.mQzssXtraAge),
    mGpsXtraValid(nmea.mGpsXtraValid),
    mGloXtraValid(nmea.mGloXtraValid),
    mBdsXtraValid(nmea.mBdsXtraValid),
    mGalXtraValid(nmea.mGalXtraValid),
    mQzssXtraValid(nmea.mQzssXtraValid)
{
}

bool SystemStatusXtra::equals(SystemStatusXtra& peer)
{
    if ((mXtraValidMask != peer.mXtraValidMask) ||
        (mGpsXtraAge != peer.mGpsXtraAge) ||
        (mGloXtraAge != peer.mGloXtraAge) ||
        (mBdsXtraAge != peer.mBdsXtraAge) ||
        (mGalXtraAge != peer.mGalXtraAge) ||
        (mQzssXtraAge != peer.mQzssXtraAge) ||
        (mGpsXtraValid != peer.mGpsXtraValid) ||
        (mGloXtraValid != peer.mGloXtraValid) ||
        (mBdsXtraValid != peer.mBdsXtraValid) ||
        (mGalXtraValid != peer.mGalXtraValid) ||
        (mQzssXtraValid != peer.mQzssXtraValid)) {
        return false;
    }
    return true;
}

void SystemStatusXtra::dump()
{
    LOC_LOGV("SystemStatusXtra: u=%ld:%ld m=%x a=%d:%d:%d:%d:%d v=%x:%x:%" PRIx64 ":%" PRIx64":%x",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mXtraValidMask,
             mGpsXtraAge,
             mGloXtraAge,
             mBdsXtraAge,
             mGalXtraAge,
             mQzssXtraAge,
             mGpsXtraValid,
             mGloXtraValid,
             mBdsXtraValid,
             mGalXtraValid,
             mQzssXtraValid);
    return;
}

/******************************************************************************
 SystemStatusEphemeris
******************************************************************************/
SystemStatusEphemeris::SystemStatusEphemeris(const SystemStatusPQWP4& nmea) :
    mGpsEpheValid(nmea.mGpsEpheValid),
    mGloEpheValid(nmea.mGloEpheValid),
    mBdsEpheValid(nmea.mBdsEpheValid),
    mGalEpheValid(nmea.mGalEpheValid),
    mQzssEpheValid(nmea.mQzssEpheValid)
{
}

bool SystemStatusEphemeris::equals(SystemStatusEphemeris& peer)
{
    if ((mGpsEpheValid != peer.mGpsEpheValid) ||
        (mGloEpheValid != peer.mGloEpheValid) ||
        (mBdsEpheValid != peer.mBdsEpheValid) ||
        (mGalEpheValid != peer.mGalEpheValid) ||
        (mQzssEpheValid != peer.mQzssEpheValid)) {
        return false;
    }
    return true;
}

void SystemStatusEphemeris::dump()
{
    LOC_LOGV("Ephemeris: u=%ld:%ld ev=%x:%x:%" PRIx64 ":%" PRIx64 ":%x",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mGpsEpheValid,
             mGloEpheValid,
             mBdsEpheValid,
             mGalEpheValid,
             mQzssEpheValid);
    return;
}

/******************************************************************************
 SystemStatusSvHealth
******************************************************************************/
SystemStatusSvHealth::SystemStatusSvHealth(const SystemStatusPQWP5& nmea) :
    mGpsUnknownMask(nmea.mGpsUnknownMask),
    mGloUnknownMask(nmea.mGloUnknownMask),
    mBdsUnknownMask(nmea.mBdsUnknownMask),
    mGalUnknownMask(nmea.mGalUnknownMask),
    mQzssUnknownMask(nmea.mQzssUnknownMask),
    mGpsGoodMask(nmea.mGpsGoodMask),
    mGloGoodMask(nmea.mGloGoodMask),
    mBdsGoodMask(nmea.mBdsGoodMask),
    mGalGoodMask(nmea.mGalGoodMask),
    mQzssGoodMask(nmea.mQzssGoodMask),
    mGpsBadMask(nmea.mGpsBadMask),
    mGloBadMask(nmea.mGloBadMask),
    mBdsBadMask(nmea.mBdsBadMask),
    mGalBadMask(nmea.mGalBadMask),
    mQzssBadMask(nmea.mQzssBadMask)
{
}

bool SystemStatusSvHealth::equals(SystemStatusSvHealth& peer)
{
    if ((mGpsUnknownMask != peer.mGpsUnknownMask) ||
        (mGloUnknownMask != peer.mGloUnknownMask) ||
        (mBdsUnknownMask != peer.mBdsUnknownMask) ||
        (mGalUnknownMask != peer.mGalUnknownMask) ||
        (mQzssUnknownMask != peer.mQzssUnknownMask) ||
        (mGpsGoodMask != peer.mGpsGoodMask) ||
        (mGloGoodMask != peer.mGloGoodMask) ||
        (mBdsGoodMask != peer.mBdsGoodMask) ||
        (mGalGoodMask != peer.mGalGoodMask) ||
        (mQzssGoodMask != peer.mQzssGoodMask) ||
        (mGpsBadMask != peer.mGpsBadMask) ||
        (mGloBadMask != peer.mGloBadMask) ||
        (mBdsBadMask != peer.mBdsBadMask) ||
        (mGalBadMask != peer.mGalBadMask) ||
        (mQzssBadMask != peer.mQzssBadMask)) {
        return false;
    }
    return true;
}

void SystemStatusSvHealth::dump()
{
    LOC_LOGV("SvHealth: u=%ld:%ld \
             u=%x:%x:%" PRIx64 ":%" PRIx64 ":%x \
             g=%x:%x:%" PRIx64 ":%" PRIx64 ":%x \
             b=%x:%x:%" PRIx64 ":%" PRIx64 ":%x",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mGpsUnknownMask,
             mGloUnknownMask,
             mBdsUnknownMask,
             mGalUnknownMask,
             mQzssUnknownMask,
             mGpsGoodMask,
             mGloGoodMask,
             mBdsGoodMask,
             mGalGoodMask,
             mQzssGoodMask,
             mGpsBadMask,
             mGloBadMask,
             mBdsBadMask,
             mGalBadMask,
             mQzssBadMask);
    return;
}

/******************************************************************************
 SystemStatusPdr
******************************************************************************/
SystemStatusPdr::SystemStatusPdr(const SystemStatusPQWP6& nmea) :
    mFixInfoMask(nmea.mFixInfoMask)
{
}

bool SystemStatusPdr::equals(SystemStatusPdr& peer)
{
    if (mFixInfoMask != peer.mFixInfoMask) {
        return false;
    }
    return true;
}

void SystemStatusPdr::dump()
{
    LOC_LOGV("Pdr: u=%ld:%ld m=%x",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mFixInfoMask);
    return;
}

/******************************************************************************
 SystemStatusNavData
******************************************************************************/
SystemStatusNavData::SystemStatusNavData(const SystemStatusPQWP7& nmea)
{
    for (uint32_t i=0; i<SV_ALL_NUM; i++) {
        mNav[i] = nmea.mNav[i];
    }
}

bool SystemStatusNavData::equals(SystemStatusNavData& peer)
{
    for (uint32_t i=0; i<SV_ALL_NUM; i++) {
        if ((mNav[i].mType != peer.mNav[i].mType) ||
            (mNav[i].mSource != peer.mNav[i].mSource) ||
            (mNav[i].mAgeSec != peer.mNav[i].mAgeSec)) {
            return false;
        }
    }
    return true;
}

void SystemStatusNavData::dump()
{
    LOC_LOGV("NavData: u=%ld:%ld",
            mUtcTime.tv_sec, mUtcTime.tv_nsec);
    for (uint32_t i=0; i<SV_ALL_NUM; i++) {
        LOC_LOGV("i=%d type=%d src=%d age=%d",
            i, mNav[i].mType, mNav[i].mSource, mNav[i].mAgeSec);
    }
    return;
}

/******************************************************************************
 SystemStatusPositionFailure
******************************************************************************/
SystemStatusPositionFailure::SystemStatusPositionFailure(const SystemStatusPQWS1& nmea) :
    mFixInfoMask(nmea.mFixInfoMask),
    mHepeLimit(nmea.mHepeLimit)
{
}

bool SystemStatusPositionFailure::equals(SystemStatusPositionFailure& peer)
{
    if ((mFixInfoMask != peer.mFixInfoMask) ||
        (mHepeLimit != peer.mHepeLimit)) {
        return false;
    }
    return true;
}

void SystemStatusPositionFailure::dump()
{
    LOC_LOGV("PositionFailure: u=%ld:%ld m=%d h=%d",
             mUtcTime.tv_sec, mUtcTime.tv_nsec,
             mFixInfoMask,
             mHepeLimit);
    return;
}

/******************************************************************************
 SystemStatusLocation
******************************************************************************/
bool SystemStatusLocation::equals(SystemStatusLocation& peer)
{
    if ((mLocation.gpsLocation.latitude != peer.mLocation.gpsLocation.latitude) ||
        (mLocation.gpsLocation.longitude != peer.mLocation.gpsLocation.longitude) ||
        (mLocation.gpsLocation.altitude != peer.mLocation.gpsLocation.altitude)) {
        return false;
    }
    return true;
}

void SystemStatusLocation::dump()
{
    LOC_LOGV("Location: lat=%f lon=%f alt=%f spd=%f",
             mLocation.gpsLocation.latitude,
             mLocation.gpsLocation.longitude,
             mLocation.gpsLocation.altitude,
             mLocation.gpsLocation.speed);
    return;
}

/******************************************************************************
 SystemStatus
******************************************************************************/
pthread_mutex_t   SystemStatus::mMutexSystemStatus = PTHREAD_MUTEX_INITIALIZER;
SystemStatus*     SystemStatus::mInstance = NULL;

SystemStatus* SystemStatus::getInstance(const MsgTask* msgTask)
{
    pthread_mutex_lock(&mMutexSystemStatus);

    if (!mInstance) {
        // Instantiating for the first time. msgTask should not be NULL
        if (msgTask == NULL) {
            LOC_LOGE("SystemStatus: msgTask is NULL!!");
            pthread_mutex_unlock(&mMutexSystemStatus);
            return NULL;
        }
        mInstance = new (nothrow) SystemStatus(msgTask);
        LOC_LOGD("SystemStatus::getInstance:%p. Msgtask:%p", mInstance, msgTask);
    }

    pthread_mutex_unlock(&mMutexSystemStatus);
    return mInstance;
}

void SystemStatus::destroyInstance()
{
    delete mInstance;
    mInstance = NULL;
}

IOsObserver* SystemStatus::getOsObserver()
{
    return &mSysStatusObsvr;
}

SystemStatus::SystemStatus(const MsgTask* msgTask) :
    mSysStatusObsvr(msgTask),
    mConnected(false)
{
    int result = 0;
    ENTRY_LOG ();
    mCache.mLocation.clear();

    mCache.mTimeAndClock.clear();
    mCache.mXoState.clear();
    mCache.mRfAndParams.clear();
    mCache.mErrRecovery.clear();

    mCache.mInjectedPosition.clear();
    mCache.mBestPosition.clear();
    mCache.mXtra.clear();
    mCache.mEphemeris.clear();
    mCache.mSvHealth.clear();
    mCache.mPdr.clear();
    mCache.mNavData.clear();

    mCache.mPositionFailure.clear();

    EXIT_LOG_WITH_ERROR ("%d",result);
}

/******************************************************************************
 SystemStatus - M1 functions
******************************************************************************/
bool SystemStatus::setTimeAndCLock(const SystemStatusPQWM1& nmea)
{
    SystemStatusTimeAndClock s(nmea);
    if (!mCache.mTimeAndClock.empty() && mCache.mTimeAndClock.back().equals(s)) {
        mCache.mTimeAndClock.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mTimeAndClock.push_back(s);
        if (mCache.mTimeAndClock.size() > maxTimeAndClock) {
            mCache.mTimeAndClock.erase(mCache.mTimeAndClock.begin());
        }
    }
    return true;
}

bool SystemStatus::setXoState(const SystemStatusPQWM1& nmea)
{
    SystemStatusXoState s(nmea);
    if (!mCache.mXoState.empty() && mCache.mXoState.back().equals(s)) {
        mCache.mXoState.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mXoState.push_back(s);
        if (mCache.mXoState.size() > maxXoState) {
            mCache.mXoState.erase(mCache.mXoState.begin());
        }
    }
    return true;
}

bool SystemStatus::setRfAndParams(const SystemStatusPQWM1& nmea)
{
    SystemStatusRfAndParams s(nmea);
    if (!mCache.mRfAndParams.empty() && mCache.mRfAndParams.back().equals(s)) {
        mCache.mRfAndParams.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mRfAndParams.push_back(s);
        if (mCache.mRfAndParams.size() > maxRfAndParams) {
            mCache.mRfAndParams.erase(mCache.mRfAndParams.begin());
        }
    }
    return true;
}

bool SystemStatus::setErrRecovery(const SystemStatusPQWM1& nmea)
{
    SystemStatusErrRecovery s(nmea);
    if (!mCache.mErrRecovery.empty() && mCache.mErrRecovery.back().equals(s)) {
        mCache.mErrRecovery.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mErrRecovery.push_back(s);
        if (mCache.mErrRecovery.size() > maxErrRecovery) {
            mCache.mErrRecovery.erase(mCache.mErrRecovery.begin());
        }
    }
    return true;
}

/******************************************************************************
 SystemStatus - Px functions
******************************************************************************/
bool SystemStatus::setInjectedPosition(const SystemStatusPQWP1& nmea)
{
    SystemStatusInjectedPosition s(nmea);
    if (!mCache.mInjectedPosition.empty() && mCache.mInjectedPosition.back().equals(s)) {
        mCache.mInjectedPosition.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mInjectedPosition.push_back(s);
        if (mCache.mInjectedPosition.size() > maxInjectedPosition) {
            mCache.mInjectedPosition.erase(mCache.mInjectedPosition.begin());
        }
    }
    return true;
}

bool SystemStatus::setBestPosition(const SystemStatusPQWP2& nmea)
{
    SystemStatusBestPosition s(nmea);
    if (!mCache.mBestPosition.empty() && mCache.mBestPosition.back().equals(s)) {
        mCache.mBestPosition.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mBestPosition.push_back(s);
        if (mCache.mBestPosition.size() > maxBestPosition) {
            mCache.mBestPosition.erase(mCache.mBestPosition.begin());
        }
    }
    return true;
}

bool SystemStatus::setXtra(const SystemStatusPQWP3& nmea)
{
    SystemStatusXtra s(nmea);
    if (!mCache.mXtra.empty() && mCache.mXtra.back().equals(s)) {
        mCache.mXtra.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mXtra.push_back(s);
        if (mCache.mXtra.size() > maxXtra) {
            mCache.mXtra.erase(mCache.mXtra.begin());
        }
    }
    return true;
}

bool SystemStatus::setEphemeris(const SystemStatusPQWP4& nmea)
{
    SystemStatusEphemeris s(nmea);
    if (!mCache.mEphemeris.empty() && mCache.mEphemeris.back().equals(s)) {
        mCache.mEphemeris.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mEphemeris.push_back(s);
        if (mCache.mEphemeris.size() > maxEphemeris) {
            mCache.mEphemeris.erase(mCache.mEphemeris.begin());
        }
    }
    return true;
}

bool SystemStatus::setSvHealth(const SystemStatusPQWP5& nmea)
{
    SystemStatusSvHealth s(nmea);
    if (!mCache.mSvHealth.empty() && mCache.mSvHealth.back().equals(s)) {
        mCache.mSvHealth.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mSvHealth.push_back(s);
        if (mCache.mSvHealth.size() > maxSvHealth) {
            mCache.mSvHealth.erase(mCache.mSvHealth.begin());
        }
    }
    return true;
}

bool SystemStatus::setPdr(const SystemStatusPQWP6& nmea)
{
    SystemStatusPdr s(nmea);
    if (!mCache.mPdr.empty() && mCache.mPdr.back().equals(s)) {
        mCache.mPdr.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mPdr.push_back(s);
        if (mCache.mPdr.size() > maxPdr) {
            mCache.mPdr.erase(mCache.mPdr.begin());
        }
    }
    return true;
}

bool SystemStatus::setNavData(const SystemStatusPQWP7& nmea)
{
    SystemStatusNavData s(nmea);
    if (!mCache.mNavData.empty() && mCache.mNavData.back().equals(s)) {
        mCache.mNavData.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mNavData.push_back(s);
        if (mCache.mNavData.size() > maxNavData) {
            mCache.mNavData.erase(mCache.mNavData.begin());
        }
    }
    return true;
}

/******************************************************************************
 SystemStatus - Sx functions
******************************************************************************/
bool SystemStatus::setPositionFailure(const SystemStatusPQWS1& nmea)
{
    SystemStatusPositionFailure s(nmea);
    if (!mCache.mPositionFailure.empty() && mCache.mPositionFailure.back().equals(s)) {
        mCache.mPositionFailure.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mPositionFailure.push_back(s);
        if (mCache.mPositionFailure.size() > maxPositionFailure) {
            mCache.mPositionFailure.erase(mCache.mPositionFailure.begin());
        }
    }
    return true;
}

/******************************************************************************
 SystemStatus - storing dataitems
******************************************************************************/
bool SystemStatus::setNetworkInfo(IDataItemCore* dataitem)
{
    SystemStatusNetworkInfo* data = reinterpret_cast<SystemStatusNetworkInfo*>(dataitem);
    SystemStatusNetworkInfo s(data->mType,data->mTypeName,data->mSubTypeName,
                              data->mAvailable,data->mConnected,data->mRoaming);
    s.dump();
    mConnected = data->mConnected;

    if (!mCache.mNetworkInfo.empty() && mCache.mNetworkInfo.back().equals(s)) {
        mCache.mNetworkInfo.back().mUtcReported = s.mUtcReported;
    } else {
        mCache.mNetworkInfo.push_back(s);
        if (mCache.mNetworkInfo.size() > maxNetworkInfo) {
            mCache.mNetworkInfo.erase(mCache.mNetworkInfo.begin());
        }
    }
    return true;
}

/******************************************************************************
@brief      API to set report data into internal buffer

@param[In]  data pointer to the NMEA string
@param[In]  len  length of the NMEA string

@return     true when successfully done
******************************************************************************/
static uint32_t cnt = 0;
static uint32_t cnt_m1 = 0;
static uint32_t cnt_p1 = 0;
static uint32_t cnt_p2 = 0;
static uint32_t cnt_p3 = 0;
static uint32_t cnt_p4 = 0;
static uint32_t cnt_p5 = 0;
static uint32_t cnt_p6 = 0;
static uint32_t cnt_p7 = 0;
static uint32_t cnt_s1 = 0;

bool SystemStatus::setNmeaString(const char *data, uint32_t len)
{
    bool ret = false;
    if (!loc_nmea_is_debug(data, len)) {
        return false;
    }

    char buf[SystemStatusNmeaBase::NMEA_MAXSIZE + 1] = { 0 };
    strlcpy(buf, data, sizeof(buf));

    pthread_mutex_lock(&mMutexSystemStatus);

    // parse the received nmea strings here
    if      (0 == strncmp(data, "$PQWM1", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        SystemStatusPQWM1 s = SystemStatusPQWM1parser(buf, len).get();
        ret  = setTimeAndCLock(s);
        ret |= setXoState(s);
        ret |= setRfAndParams(s);
        ret |= setErrRecovery(s);
        cnt_m1++;
    }
    else if (0 == strncmp(data, "$PQWP1", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setInjectedPosition(SystemStatusPQWP1parser(buf, len).get());
        cnt_p1++;
    }
    else if (0 == strncmp(data, "$PQWP2", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setBestPosition(SystemStatusPQWP2parser(buf, len).get());
        cnt_p2++;
    }
    else if (0 == strncmp(data, "$PQWP3", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setXtra(SystemStatusPQWP3parser(buf, len).get());
        cnt_p3++;
    }
    else if (0 == strncmp(data, "$PQWP4", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setEphemeris(SystemStatusPQWP4parser(buf, len).get());
        cnt_p4++;
    }
    else if (0 == strncmp(data, "$PQWP5", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setSvHealth(SystemStatusPQWP5parser(buf, len).get());
        cnt_p5++;
    }
    else if (0 == strncmp(data, "$PQWP6", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setPdr(SystemStatusPQWP6parser(buf, len).get());
        cnt_p6++;
    }
    else if (0 == strncmp(data, "$PQWP7", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setNavData(SystemStatusPQWP7parser(buf, len).get());
        cnt_p7++;
    }
    else if (0 == strncmp(data, "$PQWS1", SystemStatusNmeaBase::NMEA_MINSIZE)) {
        ret = setPositionFailure(SystemStatusPQWS1parser(buf, len).get());
        cnt_s1++;
    }
    else {
        // do nothing
    }
    cnt++;
    LOC_LOGV("setNmeaString: cnt=%d M:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d S:%d",
             cnt,
             cnt_m1,
             cnt_p1,
             cnt_p2,
             cnt_p3,
             cnt_p4,
             cnt_p5,
             cnt_p6,
             cnt_p7,
             cnt_s1);

    pthread_mutex_unlock(&mMutexSystemStatus);
    return ret;
}

/******************************************************************************
@brief      API to set report position data into internal buffer

@param[In]  UlpLocation

@return     true when successfully done
******************************************************************************/
bool SystemStatus::eventPosition(const UlpLocation& location,
                                 const GpsLocationExtended& locationEx)
{
    SystemStatusLocation s(location, locationEx);
    if (!mCache.mLocation.empty() && mCache.mLocation.back().equals(s)) {
        mCache.mLocation.back().mUtcReported = s.mUtcReported;
    }
    else {
        mCache.mLocation.push_back(s);
        if (mCache.mLocation.size() > maxLocation) {
            mCache.mLocation.erase(mCache.mLocation.begin());
        }
    }
    LOC_LOGV("eventPosition - lat=%f lon=%f alt=%f speed=%f",
             s.mLocation.gpsLocation.latitude,
             s.mLocation.gpsLocation.longitude,
             s.mLocation.gpsLocation.altitude,
             s.mLocation.gpsLocation.speed);
    return true;
}

/******************************************************************************
@brief      API to set report DataItem event into internal buffer

@param[In]  DataItem

@return     true when successfully done
******************************************************************************/
bool SystemStatus::eventDataItemNotify(IDataItemCore* dataitem)
{
    pthread_mutex_lock(&mMutexSystemStatus);
    switch(dataitem->getId())
    {
        case NETWORKINFO_DATA_ITEM_ID:
            setNetworkInfo(dataitem);
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&mMutexSystemStatus);
    return true;
}

/******************************************************************************
@brief      API to get report data into a given buffer

@param[In]  reference to report buffer
@param[In]  bool flag to identify latest only or entire buffer

@return     true when successfully done
******************************************************************************/
bool SystemStatus::getReport(SystemStatusReports& report, bool isLatestOnly) const
{
    pthread_mutex_lock(&mMutexSystemStatus);

    if (isLatestOnly) {
        // push back only the latest report and return it
        report.mLocation.clear();
        if (mCache.mLocation.size() >= 1) {
            report.mLocation.push_back(mCache.mLocation.back());
            report.mLocation.back().dump();
        }

        report.mTimeAndClock.clear();
        if (mCache.mTimeAndClock.size() >= 1) {
            report.mTimeAndClock.push_back(mCache.mTimeAndClock.back());
            report.mTimeAndClock.back().dump();
        }
        report.mXoState.clear();
        if (mCache.mXoState.size() >= 1) {
            report.mXoState.push_back(mCache.mXoState.back());
            report.mXoState.back().dump();
        }
        report.mRfAndParams.clear();
        if (mCache.mRfAndParams.size() >= 1) {
            report.mRfAndParams.push_back(mCache.mRfAndParams.back());
            report.mRfAndParams.back().dump();
        }
        report.mErrRecovery.clear();
        if (mCache.mErrRecovery.size() >= 1) {
            report.mErrRecovery.push_back(mCache.mErrRecovery.back());
            report.mErrRecovery.back().dump();
        }

        report.mInjectedPosition.clear();
        if (mCache.mInjectedPosition.size() >= 1) {
            report.mInjectedPosition.push_back(mCache.mInjectedPosition.back());
            report.mInjectedPosition.back().dump();
        }
        report.mBestPosition.clear();
        if (mCache.mBestPosition.size() >= 1) {
            report.mBestPosition.push_back(mCache.mBestPosition.back());
            report.mBestPosition.back().dump();
        }
        report.mXtra.clear();
        if (mCache.mXtra.size() >= 1) {
            report.mXtra.push_back(mCache.mXtra.back());
            report.mXtra.back().dump();
        }
        report.mEphemeris.clear();
        if (mCache.mEphemeris.size() >= 1) {
            report.mEphemeris.push_back(mCache.mEphemeris.back());
            report.mEphemeris.back().dump();
        }
        report.mSvHealth.clear();
        if (mCache.mSvHealth.size() >= 1) {
            report.mSvHealth.push_back(mCache.mSvHealth.back());
            report.mSvHealth.back().dump();
        }
        report.mPdr.clear();
        if (mCache.mPdr.size() >= 1) {
            report.mPdr.push_back(mCache.mPdr.back());
            report.mPdr.back().dump();
        }
        report.mNavData.clear();
        if (mCache.mNavData.size() >= 1) {
            report.mNavData.push_back(mCache.mNavData.back());
            report.mNavData.back().dump();
        }

        report.mPositionFailure.clear();
        if (mCache.mPositionFailure.size() >= 1) {
            report.mPositionFailure.push_back(mCache.mPositionFailure.back());
            report.mPositionFailure.back().dump();
        }
    }
    else {
        // copy entire reports and return them
        report.mLocation.clear();

        report.mTimeAndClock.clear();
        report.mXoState.clear();
        report.mRfAndParams.clear();
        report.mErrRecovery.clear();

        report.mInjectedPosition.clear();
        report.mBestPosition.clear();
        report.mXtra.clear();
        report.mEphemeris.clear();
        report.mSvHealth.clear();
        report.mPdr.clear();
        report.mNavData.clear();

        report.mPositionFailure.clear();
        report = mCache;
    }

    pthread_mutex_unlock(&mMutexSystemStatus);
    return true;
}

/******************************************************************************
@brief      API to set default report data

@param[In]  none

@return     true when successfully done
******************************************************************************/
bool SystemStatus::setDefaultReport(void)
{
    pthread_mutex_lock(&mMutexSystemStatus);

    mCache.mLocation.push_back(SystemStatusLocation());
    if (mCache.mLocation.size() > maxLocation) {
        mCache.mLocation.erase(mCache.mLocation.begin());
    }

    mCache.mTimeAndClock.push_back(SystemStatusTimeAndClock());
    if (mCache.mTimeAndClock.size() > maxTimeAndClock) {
        mCache.mTimeAndClock.erase(mCache.mTimeAndClock.begin());
    }
    mCache.mXoState.push_back(SystemStatusXoState());
    if (mCache.mXoState.size() > maxXoState) {
        mCache.mXoState.erase(mCache.mXoState.begin());
    }
    mCache.mRfAndParams.push_back(SystemStatusRfAndParams());
    if (mCache.mRfAndParams.size() > maxRfAndParams) {
        mCache.mRfAndParams.erase(mCache.mRfAndParams.begin());
    }
    mCache.mErrRecovery.push_back(SystemStatusErrRecovery());
    if (mCache.mErrRecovery.size() > maxErrRecovery) {
        mCache.mErrRecovery.erase(mCache.mErrRecovery.begin());
    }

    mCache.mInjectedPosition.push_back(SystemStatusInjectedPosition());
    if (mCache.mInjectedPosition.size() > maxInjectedPosition) {
        mCache.mInjectedPosition.erase(mCache.mInjectedPosition.begin());
    }
    mCache.mBestPosition.push_back(SystemStatusBestPosition());
    if (mCache.mBestPosition.size() > maxBestPosition) {
        mCache.mBestPosition.erase(mCache.mBestPosition.begin());
    }
    mCache.mXtra.push_back(SystemStatusXtra());
    if (mCache.mXtra.size() > maxXtra) {
        mCache.mXtra.erase(mCache.mXtra.begin());
    }
    mCache.mEphemeris.push_back(SystemStatusEphemeris());
    if (mCache.mEphemeris.size() > maxEphemeris) {
        mCache.mEphemeris.erase(mCache.mEphemeris.begin());
    }
    mCache.mSvHealth.push_back(SystemStatusSvHealth());
    if (mCache.mSvHealth.size() > maxSvHealth) {
        mCache.mSvHealth.erase(mCache.mSvHealth.begin());
    }
    mCache.mPdr.push_back(SystemStatusPdr());
    if (mCache.mPdr.size() > maxPdr) {
        mCache.mPdr.erase(mCache.mPdr.begin());
    }
    mCache.mNavData.push_back(SystemStatusNavData());
    if (mCache.mNavData.size() > maxNavData) {
        mCache.mNavData.erase(mCache.mNavData.begin());
    }

    mCache.mPositionFailure.push_back(SystemStatusPositionFailure());
    if (mCache.mPositionFailure.size() > maxPositionFailure) {
        mCache.mPositionFailure.erase(mCache.mPositionFailure.begin());
    }

    pthread_mutex_unlock(&mMutexSystemStatus);
    return true;
}

/******************************************************************************
@brief      API to handle connection status update event from GnssRil

@param[In]  Connection status

@return     true when successfully done
******************************************************************************/
bool SystemStatus::eventConnectionStatus(bool connected, uint8_t type)
{
    if (connected != mConnected) {
        mConnected = connected;

        // send networkinof dataitem to systemstatus observer clients
        SystemStatusNetworkInfo s(type, "", "", false, connected, false);
        IDataItemCore *networkinfo =
                DataItemsFactoryProxy::createNewDataItem(NETWORKINFO_DATA_ITEM_ID);
        if (nullptr == networkinfo) {
            LOC_LOGE("Unable to create dataitemd");
            return false;
        }
        networkinfo->copy(&s);
        list<IDataItemCore*> dl(0);
        dl.push_back(networkinfo);
        mSysStatusObsvr.notify(dl);
    }
    return true;
}

} // namespace loc_core

