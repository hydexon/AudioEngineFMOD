#include "FMOD_FileSystemIO.h"

#include <AzCore/Casting/numeric_cast.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/IStreamer.h>
#include <AzCore/IO/Streamer/FileRequest.h>

#include <IAudioInterfacesCommonData.h>

namespace AudioEngineFMOD
{
    struct AZIOData {
        AZStd::string filename;
        AZ::IO::FileRequestPtr streamingRequest;
        std::atomic<bool> completedStreamerRequest = false;
    };

    static AZ::IO::HandleType GetRealFileHandle(void* ptr)
    {
        return static_cast<AZ::IO::HandleType>(reinterpret_cast<uintptr_t>(ptr));
    }

    static void* SetOpaqueHandle(AZ::IO::HandleType handle)
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    }

    namespace SyncIO
    {

    FMOD_RESULT AzFileOpen(const char *name, unsigned int *filesize, void **handle, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        if(AZ::u64 fileSize = 0; fileIO->Size(name, fileSize) && fileSize != 0)
        {
            AZ::IO::HandleType fileHandle = AZ::IO::InvalidHandle;
            fileIO->Open(name, AZ::IO::OpenMode::ModeBinary, fileHandle);
            if(fileHandle != AZ::IO::InvalidHandle)
            {
                *filesize = aznumeric_cast<unsigned int>(fileSize); //We assume we don't load giantic bank files.
                *handle = SetOpaqueHandle(fileHandle);

                AZIOData* IOData = azcreate(AZIOData);
                IOData->filename = name;
                IOData->streamingRequest = nullptr;
                userData = IOData;

                return FMOD_OK;

            }
        }

        return FMOD_ERR_FILE_NOTFOUND;
    }

    FMOD_RESULT AzFileClose(void *handle, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto fileHandle = GetRealFileHandle(handle);
        if(fileIO->Close(fileHandle))
        {
            if(userData)
            {
                auto iodata = reinterpret_cast<AZIOData*>(userData);
                azdestroy(iodata);
            }

            return FMOD_OK;
        }

        return FMOD_ERR_FILE_BAD;
    }

    FMOD_RESULT AzFileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesRead, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto fileHandle = GetRealFileHandle(handle);

        AZ::u64 bytesRead64 = 0;
        fileIO->Read(fileHandle, buffer, aznumeric_cast<AZ::u64>(sizebytes), false, &bytesRead64);
        const bool readOk = (bytesRead64 == aznumeric_cast<AZ::u64>(sizebytes));

        AZ_Assert(readOk, "Number of bytes read (%llu) for read request doesn't match the requested size (%u)", bytesRead64, sizebytes);
        *bytesRead = aznumeric_cast<unsigned int>(bytesRead64);

        return readOk ? FMOD_OK : FMOD_ERR_FILE_BAD;
    }

    FMOD_RESULT AzFileSeek(void *handle, unsigned int pos, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto fileHandle = GetRealFileHandle(handle);

        const bool seekOk = fileIO->Seek(fileHandle, aznumeric_cast<AZ::u64>(pos), AZ::IO::SeekType::SeekFromStart);
        return seekOk ? FMOD_OK : FMOD_ERR_FILE_COULDNOTSEEK;
    }

    }

    namespace AsyncIO
    {

    FMOD_RESULT AzAsyncFileRead(FMOD_ASYNCREADINFO *info, void *userData)
    {
        auto streamer = AZ::Interface<AZ::IO::IStreamer>::Get();
        AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);
        AZStd::string& filename = IOData->filename;
        AZ::u8 priority = aznumeric_caster( //Thank god FMOD and Wwise priority ranges is the same.
              (info->priority << 1) // 100 -> 200
            + (info->priority >> 1) // 200 -> 250
            + (info->priority >> 4) // 250 -> 256
            - (info->priority >> 6));  // 256 -> 255

        AZ::IO::FileRequestPtr request = streamer->Read(filename,
                                                        info->buffer,
                                                        info->sizebytes,
                                                        info->sizebytes,
                                                        AZ::IO::IStreamerTypes::s_noDeadline,
                                                        priority,
                                                        info->offset);

        streamer->SetRequestCompleteCallback(request, [info](AZ::IO::FileRequestPtr request) {
            AZ::IO::IStreamerTypes::RequestStatus status = AZ::Interface<AZ::IO::IStreamer>::Get()->GetRequestStatus(request);
            AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);

            switch (status) {
            case AZ::IO::IStreamerTypes::RequestStatus::Completed:
                info->done(info, FMOD_OK);
                IOData->completedStreamerRequest = true;
                break;
            case AZ::IO::IStreamerTypes::RequestStatus::Canceled:
                //Uh oh.
                AZ_Trace("FMODAudioSystem", "Streamer File Request for file %s has been canceled", IOData->filename.c_str());
                break;
            default:
                info->done(info, FMOD_ERR_FILE_BAD);
                break;
            }

        });
        IOData->streamingRequest = request;
        streamer->QueueRequest(IOData->streamingRequest);
        return FMOD_OK;
    }

    FMOD_RESULT AzAsyncFileCancel(FMOD_ASYNCREADINFO *info, void *userData)
    {
        auto streamer = AZ::Interface<AZ::IO::IStreamer>::Get();
        AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);

        if(!IOData->completedStreamerRequest)
        {
            auto cancelRequest = streamer->Cancel(IOData->streamingRequest);
            streamer->QueueRequest(cancelRequest);
        }

        return FMOD_OK;
    }

    }


}