#pragma once
#include <vector>

struct MainHeader
{
    unsigned char ID[4];
    int HeaderSize;
    int SectorSize;
    int CRC1;
    __int64 MFTOffset;
    int MFTSize;
    int Flags;
};

struct MFTHeader
{
    unsigned char ID[4];
    int Unk1;
    int Unk2;
    int EntryCount;
    int Unk4;
    int Unk5;
};

enum FileType
{
    AMAT,
    AMP,
    ATEXDXT1,
    ATEXDXT2,
    ATEXDXT3,
    ATEXDXT4,
    ATEXDXT5,
    ATEXDXTN,
    ATEXDXTA,
    ATEXDXTL,
    ATTXDXT1,
    ATTXDXT3,
    ATTXDXT5,
    ATTXDXTN,
    ATTXDXTA,
    ATTXDXTL,
    DDS,
    FFNA,
    MFTBASE,
    NOTREAD,
    SOUND,
    TEXT,
    UNKNOWN
};

struct MFTEntry
{
    __int64 Offset;
    __int32 Size;
    unsigned short a;
    unsigned char b;
    unsigned char c;
    __int32 ID;
    __int32 CRC;
    __int32 type;
    __int32 uncompressedSize;
    __int32 Hash;
};

struct MFTExpansion
{
    int FileNumber;
    int FileOffset;
};

class GWDat
{
public:
    unsigned int readDat(const TCHAR* file);
    unsigned char* readFile(unsigned int n, bool translate = true);

    MFTEntry& operator[](const int n) { return MFT[n]; }

    MFTEntry* get_MFT_entry_ptr(const int n) { return &MFT[n]; }
    std::vector<MFTEntry>& get_MFT() { return MFT; }

    int getSectorSize() { return GWHead.SectorSize; }

    unsigned int getNumFiles() { return (unsigned int)MFT.size(); }

    void sort(unsigned int* index, int column, bool ascending);

    GWDat()
        : fileHandle(0)
    {
        InitializeCriticalSection(&criticalSection);
    };

    ~GWDat()
    {
        if (fileHandle)
            CloseHandle(fileHandle);

        DeleteCriticalSection(&criticalSection);
    }

    unsigned int getFilesRead() const { return filesRead; }
    unsigned int getTextureFiles() const { return textureFiles; }
    unsigned int getSoundFiles() const { return soundFiles; }
    unsigned int getFfnaFiles() const { return ffnaFiles; }
    unsigned int getUnknownFiles() const { return unknownFiles; }
    unsigned int getTextFiles() const { return textFiles; }
    unsigned int getMftBaseFiles() const { return mftBaseFiles; }
    unsigned int getAmatFiles() const { return amatFiles; }

protected:
    HANDLE fileHandle;
    MainHeader GWHead;
    MFTHeader MFTH;
    std::vector<MFTExpansion> MFTX;
    std::vector<MFTEntry> MFT;

    //Critical Section to make readFile thread safe
    CRITICAL_SECTION criticalSection;

    //Counters for statistics
    unsigned int filesRead;
    unsigned int textureFiles;
    unsigned int soundFiles;
    unsigned int ffnaFiles;
    unsigned int unknownFiles;
    unsigned int textFiles;
    unsigned int mftBaseFiles;
    unsigned int amatFiles;

protected:
    //wrappers for the OS seek and read functions
    void seek(__int64 offset, int origin);
    void read(void* buffer, int size, int count);
};

std::string typeToString(int type);
