#define UNICODE

#include <windows.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

int main()
{
    vector<DISPLAYCONFIG_PATH_INFO> paths;
    vector<DISPLAYCONFIG_MODE_INFO> modes;
    UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
    LONG result = ERROR_SUCCESS;

    UINT32 pathCount, modeCount;

    do {
        // Determine how many path and mode structures to allocate
        result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        // Allocate the path and mode arrays
        paths.resize(pathCount);
        modes.resize(modeCount);

        // Get all active paths and their modes
        result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

        // The function may have returned fewer paths/modes than estimated
        paths.resize(pathCount);
        modes.resize(modeCount);

        // It's possible that between the call to GetDisplayConfigBufferSizes and QueryDisplayConfig
        // that the display state changed, so loop on the case of ERROR_INSUFFICIENT_BUFFER.
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(result);
    }

    // For each active path
    for (auto& path : paths)
    {
        // Find the target (monitor) friendly name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = path.targetInfo.adapterId;
        targetName.header.id = path.targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);

        result = DisplayConfigGetDeviceInfo(&targetName.header);

        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        // Find the adapter device name
        DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
        adapterName.header.adapterId = path.targetInfo.adapterId;
        adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
        adapterName.header.size = sizeof(adapterName);

        result = DisplayConfigGetDeviceInfo(&adapterName.header);

        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }


        path.targetInfo.refreshRate.Numerator = 60;
        path.targetInfo.refreshRate.Denominator = 1;

        wcout << "Refresh: " << path.targetInfo.refreshRate.Numerator << " / " << path.targetInfo.refreshRate.Denominator << endl;
        wcout << "Scanline: " << path.targetInfo.scanLineOrdering << endl;
        wcout << "Target Available: " << path.targetInfo.targetAvailable << endl;
    }

    result = SetDisplayConfig(pathCount, paths.data(), modeCount, NULL, SDC_APPLY);

    wcout << result << endl;
    

    /*
    // For each active mode
    for (auto& mode : modes)
    {
        wcout << "Type: " << mode.infoType << endl;
            //DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE = 1;
            //DISPLAYCONFIG_MODE_INFO_TYPE_TARGET = 2;
            //DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE = 3;

            if (mode.infoType & DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
                wcout << mode.targetMode.targetVideoSignalInfo.activeSize.cx << " x " << mode.targetMode.targetVideoSignalInfo.activeSize.cx << endl;
                wcout << mode.targetMode.targetVideoSignalInfo.vSyncFreq.Numerator << " : " << mode.targetMode.targetVideoSignalInfo.vSyncFreq.Denominator << endl;
                wcout << mode.targetMode.targetVideoSignalInfo.hSyncFreq.Numerator / mode.targetMode.targetVideoSignalInfo.hSyncFreq.Denominator << endl;
                wcout << mode.targetMode.targetVideoSignalInfo.videoStandard << endl; // TODO: look at other values
            } else if (mode.infoType & DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
                wcout << mode.sourceMode.width << " x " << mode.sourceMode.height << endl;
            }
        wcout << endl;
    }
    */
}