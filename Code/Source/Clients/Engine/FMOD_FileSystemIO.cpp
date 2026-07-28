#include "FMOD_FileSystemIO.h"

#include <AzCore/Casting/numeric_cast.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/IStreamer.h>
#include <AzCore/IO/Streamer/FileRequest.h>

#include <IAudioInterfacesCommonData.h>
#include "ConfigFMOD.h"

namespace AudioEngineFMOD
{
    struct AZIOData {
        AZ::u64 fileSize;
        AZ::IO::HandleType handle;
        AZStd::string filename;
        AZ::IO::FileRequestPtr streamingRequest;
        std::atomic<bool> completedStreamerRequest = false;
    };


    namespace SyncIO
    {

    FMOD_RESULT AzFileOpen(const char *name, unsigned int *filesize, void **handle, void *userData)
    {
        const AZ::IO::FixedMaxPath AssetsBankPath = GetFMODBanksRootPath().data();
        auto fullFilePath = AZ::IO::FixedMaxPath { AssetsBankPath / name }.Native();

        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        if(AZ::u64 fileSize = 0; fileIO->Size(fullFilePath.data(), fileSize) && fileSize != 0)
        {
            AZ::IO::HandleType fileHandle = AZ::IO::InvalidHandle;
            if(fileIO->Open(fullFilePath.data(), AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary, fileHandle))
            {
                *filesize = aznumeric_cast<unsigned int>(fileSize); //We assume we don't load gigantic bank files (<2GB).
                AZIOData* IOData = azcreate(AZIOData);
                IOData->fileSize = fileSize;
                IOData->filename = fullFilePath;
                IOData->streamingRequest = nullptr;
                IOData->handle = fileHandle;
                *handle = IOData;

                return FMOD_OK;
            }
        }
        AZ_Error("FMODAudioSystem", false, "Oops FMOD IO requested file: %s, couldn't be found", fullFilePath.c_str());
        return FMOD_ERR_FILE_NOTFOUND;
    }

    FMOD_RESULT AzFileClose(void *handle, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto iodata = reinterpret_cast<AZIOData*>(handle);

        if(!iodata)
        {
            return FMOD_ERR_FILE_BAD;
        }

        if(fileIO->Close(iodata->handle))
        {
            azdestroy(iodata);
            return FMOD_OK;
        }

        return FMOD_ERR_FILE_BAD;
    }

    FMOD_RESULT AzFileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesRead, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto iodata = reinterpret_cast<AZIOData*>(handle);

        if(!iodata)
        {
            return FMOD_ERR_INVALID_HANDLE;
        }

        AZ::u64 bytesRead64 = 0;
        fileIO->Read(iodata->handle, buffer, aznumeric_cast<AZ::u64>(sizebytes), true, &bytesRead64);
        *bytesRead = aznumeric_cast<unsigned int>(bytesRead64);

        if(bytesRead64 < sizebytes)
        {
            if(fileIO->Eof(iodata->handle)) {
                return FMOD_ERR_FILE_EOF;
            }
            return FMOD_ERR_FILE_BAD;
        }

        return FMOD_OK;
    }

    FMOD_RESULT AzFileSeek(void *handle, unsigned int pos, void *userData)
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        auto iodata = reinterpret_cast<AZIOData*>(handle);

        const bool seekOk = fileIO->Seek(iodata->handle, aznumeric_cast<AZ::s64>(pos), AZ::IO::SeekType::SeekFromStart);
        return seekOk ? FMOD_OK : FMOD_ERR_FILE_COULDNOTSEEK;
    }

    }

    namespace AsyncIO
    {

    FMOD_RESULT AzAsyncFileRead(FMOD_ASYNCREADINFO *info, void *userData)
    {
        AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);
        if(!IOData)
        {
            return FMOD_ERR_INVALID_HANDLE;
        }
        auto streamer = AZ::Interface<AZ::IO::IStreamer>::Get();

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

        auto callback = [&info](AZ::IO::FileRequestHandle request) {
            AZ::IO::IStreamerTypes::RequestStatus status = AZ::Interface<AZ::IO::IStreamer>::Get()->GetRequestStatus(request);
            AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);
            if(!IOData)
            {
                info->done(info, FMOD_ERR_INVALID_HANDLE);
                return;
            }

            switch (status) {
            case AZ::IO::IStreamerTypes::RequestStatus::Completed:
                IOData->completedStreamerRequest = true;
                info->done(info, FMOD_OK);
                break;
            case AZ::IO::IStreamerTypes::RequestStatus::Canceled:
                //Uh oh.
                AZ_Trace("FMODAudioSystem", "Streamer File Request for file %s has been canceled", IOData->filename.c_str());
                break;
            default:
                info->done(info, FMOD_ERR_FILE_BAD);
                break;
            }
        };

        streamer->SetRequestCompleteCallback(request, std::move(callback));
        IOData->streamingRequest = request;
        streamer->QueueRequest(IOData->streamingRequest);
        return FMOD_OK;
    }

    FMOD_RESULT AzAsyncFileCancel(FMOD_ASYNCREADINFO *info, void *userData)
    {
        auto streamer = AZ::Interface<AZ::IO::IStreamer>::Get();
        AZIOData* IOData = reinterpret_cast<AZIOData*>(info->userdata);
        if(!IOData)
        {
            return FMOD_ERR_INVALID_HANDLE;
        }

        if(!IOData->completedStreamerRequest)
        {
            auto cancelRequest = streamer->Cancel(IOData->streamingRequest);
            streamer->QueueRequest(cancelRequest);
        }

        return FMOD_OK;
    }

    }


}