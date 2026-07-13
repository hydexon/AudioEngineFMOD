#pragma once

#include <fmod_studio.hpp>

namespace AudioEngineFMOD
{
    namespace SyncIO
    {
        FMOD_RESULT F_CALL AzFileOpen(const char* name, unsigned int *filesize, void** handle, void* userData);
        FMOD_RESULT F_CALL AzFileClose(void* handle, void* userData);
        FMOD_RESULT F_CALL AzFileRead(void* handle, void* buffer, unsigned int sizebytes, unsigned int *bytesRead, void* userData);
        FMOD_RESULT F_CALL AzFileSeek(void* handle, unsigned int pos, void* userData);
    }

    namespace AsyncIO
    {
        FMOD_RESULT F_CALL AzAsyncFileRead(FMOD_ASYNCREADINFO* info, void* userData);
        FMOD_RESULT F_CALL AzAsyncFileCancel(FMOD_ASYNCREADINFO* info, void* userData);

    }
}