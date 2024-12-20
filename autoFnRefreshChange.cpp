#define UNICODE

#include <Windows.h>
#include <iostream>

#include <chrono>
#include <thread>

using namespace std;

// TODO: Add ability to switch between two modes: 1. Legion Fn+R Refresh Active Mode Fix, and 2. Custom Hotkey Mode (for non Legion laptops)
enum MODE_TYPE {
  LENOVO_FN_REFRESH_FIX,
  CUSTOM_HOTKEY_MODE
};

MODE_TYPE mode = LENOVO_FN_REFRESH_FIX;

// Display Manipulation Variables
UINT32 numPathArrayElements = 0, numModeInfoArrayElements = 0;
UINT32 filter = QDC_ONLY_ACTIVE_PATHS;

DISPLAYCONFIG_PATH_INFO* pathArray;
DISPLAYCONFIG_MODE_INFO* modeInfoArray;

int sourceIx;
int targetIx;

// Keyboard Event (Hotkey Mode) Variables
HHOOK hHook;


// TODO: Currently Unused Functions (useful for debugging)
/*
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
int printDisplayConfig() {
    cout << "Target modeInfoIdx: " << sourceIx << endl;
    cout << "Source modeInfoIdx: " << targetIx << endl;

    // Get Target and Source Mode Info
    wcout << "SOURCE MODE INFO" << endl;
    wcout << "\tResolution: " << modeInfoArray[sourceIx].sourceMode.width << " x " << modeInfoArray[sourceIx].sourceMode.height << endl;

    wcout << "TARGET MODE INFO" << endl;
    wcout << "\tActive Size: " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.activeSize.cx << " x " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.activeSize.cy << endl;
    wcout << "\tRefresh Rate: " << modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Numerator / modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Denominator << endl;
}
*/

// TODO: Implement CUSTOM_HOTKEY_MODE
/*
// TODO: Use this to capture a key stroke combination (hotkey)
// Callback Function that Detects LL Keyboard Event
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam ) {
    char pressedKey;

    // Declare a pointer to the KBDLLHOOKSTRUCTdsad
    KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
    pressedKey = (char)pKeyBoard->vkCode; // get the key
    
    switch( wParam ) {
        case WM_KEYDOWN:
            //pressedKey = (char)pKeyBoard->vkCode; // get the key
            break;
        case WM_KEYUP: // When the key has been pressed and released
            pressedKey = (char)pKeyBoard->vkCode; // get the key
            cout << "Key pressed: " << pressedKey << endl;
            break;
        default:
            break;
    }

    //cout << "Key pressed: " << pressedKey << endl;

    // All functions which implement a hook must return by calling next hook
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
*/


/* ~~ Display Config Functions ~~ */
// Initialize the Display Configs for Available Displays
int getDisplayConfigs() {
    // TODO: Handle errors correctly

    LONG result;

    // Get Buffer Sizes and Initialize Path and Mode arrays 
    result = GetDisplayConfigBufferSizes(filter, &numPathArrayElements, &numModeInfoArrayElements);
    if (result != 0) {
        return result;
    }
    
    pathArray = new DISPLAYCONFIG_PATH_INFO[numPathArrayElements];
    modeInfoArray = new DISPLAYCONFIG_MODE_INFO[numModeInfoArrayElements];

    // Fill Path and Mode Arrays with Display Device Configs
    result = QueryDisplayConfig(filter, &numPathArrayElements, pathArray, &numModeInfoArrayElements, modeInfoArray, NULL);
    if (result != 0) {
        return result;
    }

    return 0;
}

// Select the Display (Path Index) to View/Manipulate sourceIx and targetIx
int selectDisplayPathIndex(int index = 0) {
    // Assuming path[0] is primary 
    // TODO: Add check for this
    // TODO:    Idea: On startup, save the id of 0 as primary and then check to make sure that we are changing primary
    // TODO:    This may not be a problem. BEFORE INVESTING TOO MUCH TIME INTO THIS, see what happens when you are connected
    //          to multiple monitors and run this program as it is.

    if (index > numPathArrayElements-1) {
        cerr << "Path Array Index Out of Bounds in Function: selectDisplayPathIndex" << endl;
        return -1;
    }
    
    sourceIx = pathArray[index].sourceInfo.modeInfoIdx;
    targetIx = pathArray[index].targetInfo.modeInfoIdx;

    return 0;
}

// Get Display Config Refresh of Selected Display
int getDisplayConfigRefresh() {
    return (int)(modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Numerator / modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Denominator);
}

// Set Display Config Refresh of Selected Display (Change Target Mode and Apply)
int setDisplayConfigRefresh(int newRefreshRate) {
    // Note:   TargetMode must be changed in order to cause display adapter refresh.
    //         SourceMode changes display but not the active signal. ...Exactly what I was struggling with.
    //             e.g. modeInfoArray[sourceIx].sourceMode.width = 1280;  // The sourceMode change affects the monitor but not the active signal
    modeInfoArray[targetIx].targetMode.targetVideoSignalInfo.vSyncFreq.Numerator = newRefreshRate;

    return SetDisplayConfig(numPathArrayElements, pathArray, numModeInfoArrayElements, modeInfoArray, SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES | SDC_SAVE_TO_DATABASE);
}

// Detects if Display Updated to 60Hz and Reupdates if so.
int displayUpdate() {
    this_thread::sleep_for(chrono::milliseconds(500));

    // Need to refresh display configs to get updated information
    getDisplayConfigs();

    if (getDisplayConfigRefresh() == 60) {
        setDisplayConfigRefresh(60);
        return 0;
    } else {
        return 1;
    }
}

/* ~~ LENOVO_FN_REFRESH_FIX Functions ~~ */
// Callback Function that Detects Display Change Event
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DISPLAYCHANGE:
            displayUpdate();
            break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int initWindow() {
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Auto Refresh Change Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    HWND hWnd = CreateWindowEx(
        WS_EX_NOREDIRECTIONBITMAP,              // Optional window styles.  // Previous Value: 0
        CLASS_NAME,                             // Window class
        L"Auto Fn Refresh Change Dummy Window", // Window text              // TODO: This can be NULL
        WS_DISABLED,                            // Window style
        0, 0, 0, 0,                             // Size and position
        NULL,   // Parent window    
        NULL,   // Menuk
        NULL,   // Instance handle      // Previous Value: wc.hInstance,
        NULL    // Additional application data
        );

    if (hWnd == NULL) {
        return -1;
    }

    return 0;
}


int main() {
    switch (mode) {
    case LENOVO_FN_REFRESH_FIX:
        initWindow();
        break;
    case CUSTOM_HOTKEY_MODE:
        // TODO: Implement custom hotkey mode
            // Set a global Windows Hook to capture keystrokes using the LowLevelKeyboardProc function defined above
            //hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        break;
    default:
        cerr << "Not A Valid Mode..." << endl;
        break;
    }
    
    getDisplayConfigs();
    selectDisplayPathIndex();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) continue;
    

    // TODO: Handle termination of application
        // UnhookWindowsHookEx(hHook); // Unhook the LL keyboard event hook
    return 0;
}