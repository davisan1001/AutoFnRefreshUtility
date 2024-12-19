#define UNICODE

#include <Windows.h>
#include <iostream>

using namespace std;

// TODO: Add ability to switch between two modes: 1. Legion Fn+R Refresh Active Mode Fix, and 2. Custom Hotkey Mode (for non Legion laptops)

UINT32 numPathArrayElements = 0, numModeInfoArrayElements = 0;
UINT32 filter = QDC_ONLY_ACTIVE_PATHS;

DISPLAYCONFIG_PATH_INFO* pathArray;
DISPLAYCONFIG_MODE_INFO* modeInfoArray;

// Return String of DisplayConfig Error Codes
CHAR* getDisplayConfigError(LONG error) {
    switch (error) {
    case ERROR_INVALID_PARAMETER:
        return "ERROR_INVALID_PARAMETER";
        break;
    case ERROR_NOT_SUPPORTED:
        return "ERROR_NOT_SUPPORTED";
        break;
    case ERROR_ACCESS_DENIED:
        return "ERROR_ACCESS_DENIED";
        break;
    case ERROR_GEN_FAILURE:
        return "ERROR_GEN_FAILURE";
        break;
    case ERROR_INSUFFICIENT_BUFFER:
        return "ERROR_INSUFFICIENT_BUFFER";
        break;
    default:
        return "UNKOWN_ERROR";
        break;
    }
}

// Print All Display Devices Gathered in Path Array
int printAllDisplayDevices() {
    // List each path
    for (size_t i = 0; i < numPathArrayElements; i++) {
        LONG result = ERROR_SUCCESS;

        // Find the target (monitor) friendly name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = pathArray[i].targetInfo.adapterId; // TODO: This should probably not be adapterId
        targetName.header.id = pathArray[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        result = DisplayConfigGetDeviceInfo(&targetName.header);

        if (result != ERROR_SUCCESS) {
            cout << targetName.header.id << endl;
            cout << "Error: " << getDisplayConfigError(result) << endl;
            return HRESULT_FROM_WIN32(result);
        }

        // Find the adapter device name
        DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
        adapterName.header.adapterId = pathArray[i].targetInfo.adapterId;
        adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
        adapterName.header.size = sizeof(adapterName);

        result = DisplayConfigGetDeviceInfo(&adapterName.header);

        if (result != ERROR_SUCCESS) {
            cout << "Error: " << getDisplayConfigError(result) << endl;
            return HRESULT_FROM_WIN32(result);
        }

        wcout
            << L"Monitor with name: "
            << targetName.monitorDevicePath << "\n"
            //<< (targetName.flags.friendlyNameFromEdid ? targetName.monitorFriendlyDeviceName : L"Unknown") << "\n"
            << L"is connected to adapter: "
            << adapterName.adapterDevicePath << "\n"
            << L"on target "
            << pathArray[i].targetInfo.id
            << L"\n\n";
    }
}


int main() {
    // Get Buffer Sizes and Initialize Path and Mode arrays 
    GetDisplayConfigBufferSizes(filter, &numPathArrayElements, &numModeInfoArrayElements);

    pathArray = new DISPLAYCONFIG_PATH_INFO[numPathArrayElements];
    modeInfoArray = new DISPLAYCONFIG_MODE_INFO[numModeInfoArrayElements];
    // TODO: Are these necessary?
    //ZeroMemory(pathArray, sizeof(DISPLAYCONFIG_PATH_INFO)* numPathArrayElements);
    //ZeroMemory(modeInfoArray, sizeof(DISPLAYCONFIG_MODE_INFO)* numModeInfoArrayElements);

    // Fill Path and Mode Arrays with Display Device Configs
    QueryDisplayConfig(filter, &numPathArrayElements, pathArray, &numModeInfoArrayElements, modeInfoArray, NULL);    

    // Print All Display Devices Gathered in Path Array
    printAllDisplayDevices();

    //assuming path[0] is primary 
    // TODO: Add check for this
    // TODO:    Idea: On startup, save the id of 0 as primary and then check to make sure that we are changing primary
    // TODO:    This may not be a problem. BEFORE INVESTING TOO MUCH TIME INTO THIS, see what happens when you are connected
    //          to multiple monitors and run this program as it is.
    int sourceIx = pathArray[0].sourceInfo.modeInfoIdx;
    int targetIx = pathArray[0].targetInfo.modeInfoIdx;

    cout << "Target modeInfoIdx: " << sourceIx << endl;
    cout << "Source modeInfoIdx: " << targetIx << endl;

    // Get Target and Source Mode Info
    wcout << "SOURCE MODE INFO" << endl;
    wcout << "\tResolution: " << modeInfoArray[sourceIx].sourceMode.width << " x " << modeInfoArray[sourceIx].sourceMode.height << endl;

    wcout << "TARGET MODE INFO" << endl;
    wcout << "\tActive Size: " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.activeSize.cx << " x " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.activeSize.cy << endl;
    wcout << "\tRefresh Rate: " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Numerator / modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Denominator << endl;

    
    // Set Display Config (Change Target Mode and Apply)
    //      Note:   TargetMode must be changed in order to cause display adapter refresh.
    //              SourceMode changes display but not the active signal. ...Exactly what I was struggling with.
    //                  e.g. modeInfoArray[sourceIx].sourceMode.width = 1280;  // The sourceMode change affects the monitor but not the active signal
    modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Numerator = 60;

    SetDisplayConfig(numPathArrayElements, pathArray, numModeInfoArrayElements, modeInfoArray, SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES | SDC_SAVE_TO_DATABASE);

    return 0;
}