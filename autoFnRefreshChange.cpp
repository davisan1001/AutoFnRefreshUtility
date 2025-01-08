#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>

#include <iostream>
#include <vector>

#include <chrono>
#include <thread>

using namespace std;


/* ~~~ VARIABLES ~~~ */

enum MODE_TYPE {
  LENOVO_FN_REFRESH_FIX,
  CUSTOM_HOTKEY_MODE
};

MODE_TYPE mode = LENOVO_FN_REFRESH_FIX;     // USER: Choose which mode to run the application in.

const int REFRESH_RATE_1 = 60;              // USER: Modify these for your own setup
const int REFRESH_RATE_2 = 165;             // USER: Modify these for your own setup

/* ~~ Window Variables ~~ */
HWND hWnd;

/* ~~ Display Manipulation Variables ~~ */
UINT32 numPathArrayElements = 0, numModeInfoArrayElements = 0;
UINT32 filter = QDC_ONLY_ACTIVE_PATHS;

vector<DISPLAYCONFIG_PATH_INFO> pathArray;
vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray;

int sourceIx;
int targetIx;

/* ~~ Lenovo Fn+R Fix Variables ~~ */
// Value in milliseconds to sleep before fixing display refresh.
//  This value may or may not be needed depending on how your computer reacts to
//  the display update change. If you notice strange behaviour, try adding some delay.
//  Default is 0;
int displayRefreshSleep = 0;    // USER: Modify these for your own setup

// Keep track of the refresh rate of the device.
//      This is used to make sure we only force an update when the refresh rate changes
//      from REFRESH_RATE_2 to REFRESH_RATE_1 (i.e. only as a result of the Fn+R key.).
//      This is to make sure we don't force an extra update when a different kind of
//      display change (not refresh change) is detected.
int currentRefreshRate;


/* ~~~ FUNCTIONS ~~~ */

/* ~~ Keyboard Event (Hotkey Mode) Variables ~~ */
const int REFRESH_CHANGE_HOTKEY = 1000;
UINT hotkeyMod = (MOD_WIN + MOD_CONTROL) | MOD_NOREPEAT;
UINT hotkeyVk = (UINT)'R';


/* ~~ Display Config Functions ~~ */
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

// Initialize the Display Configs for Available Displays
int getDisplayConfigs() {
    LONG result = ERROR_SUCCESS;
    do {
        // Get Buffer Sizes and Initialize Path and Mode arrays 
        result = GetDisplayConfigBufferSizes(filter, &numPathArrayElements, &numModeInfoArrayElements);
        if (result != ERROR_SUCCESS) {
            return result;
        }
        
        // Allocate the path and mode arrays
        pathArray.resize(numPathArrayElements);
        modeInfoArray.resize(numModeInfoArrayElements);

        // Fill Path and Mode Arrays with Display Device Configs
        result = QueryDisplayConfig(filter, &numPathArrayElements, pathArray.data(), &numModeInfoArrayElements, modeInfoArray.data(), NULL);
        
        // The function may have returned fewer paths/modes than estimated
        pathArray.resize(numPathArrayElements);
        modeInfoArray.resize(numModeInfoArrayElements);
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS) {
        return result;
    }

    return result; // Should be ERROR_SUCCESS at this point.
}

// Select the Display (Path Index) to View/Manipulate sourceIx and targetIx
int selectDisplayPathIndex(int index = 0) {
    // Assuming path[0] is primary 
    // TODO: Add check for this...
    //          This may not be a problem. BEFORE INVESTING TOO MUCH TIME INTO THIS, see what happens when you are connected
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

    return SetDisplayConfig(numPathArrayElements, pathArray.data(), numModeInfoArrayElements, modeInfoArray.data(), SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES | SDC_SAVE_TO_DATABASE);
}

/* ~~ LENOVO_FN_REFRESH_FIX Functions ~~ */
// Detects if Display Updated to 60Hz and Reupdates if so. Ignores Updates to 165Hz.
int lenovoFnFixDisplayUpdate() {
    if (displayRefreshSleep != 0) {
        this_thread::sleep_for(chrono::milliseconds(displayRefreshSleep));
    }

    // Need to refresh display configs to get updated information
    LONG result = getDisplayConfigs();
    if (result != ERROR_SUCCESS) {
        cerr << "Error Getting Display Configs: " << getDisplayConfigError(result) << endl;
        return -1;
    }

    if (getDisplayConfigRefresh() == REFRESH_RATE_1 && currentRefreshRate == REFRESH_RATE_2) {
        setDisplayConfigRefresh(REFRESH_RATE_1);
    }

    return 0;
}

/* ~~ CUSTOM_HOTKEY_MODE Functions ~~ */
int customHotkeyFixDisplayUpdate() {
    LONG result = getDisplayConfigs();
    if (result != ERROR_SUCCESS) {
        cerr << "Error Getting Display Configs: " << getDisplayConfigError(result) << endl;
        return -1; 
    }

    switch (getDisplayConfigRefresh()) {
    case REFRESH_RATE_1:
        setDisplayConfigRefresh(REFRESH_RATE_2);
        break;
    case REFRESH_RATE_2:
        setDisplayConfigRefresh(REFRESH_RATE_1);
        break;
    default:
        // Neither Refresh Rate is Set... Set to REFRESH_RATE_1 as Default
        setDisplayConfigRefresh(REFRESH_RATE_1);
        break;
    }

    return 0;
}

/* ~~ Main Application Functions ~~ */
// Window Process Callback Function
//      Detects Display Change Events for LENOVO_FN_REFRESH_FIX mode.
//      Handles Close and Shutdown Messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DISPLAYCHANGE:
            if (mode == LENOVO_FN_REFRESH_FIX) {
                lenovoFnFixDisplayUpdate();
            }
            // Update Refresh Rate Tracking Variable
            this_thread::sleep_for(chrono::milliseconds(50)); // Wait a short time to make sure the display refresh has updated.
            getDisplayConfigs();
            currentRefreshRate = getDisplayConfigRefresh();
            break;
        case WM_QUERYENDSESSION:
            return TRUE; // Agree to termination
            break;
        case WM_ENDSESSION:
            // Handle Termination
            if (wParam == TRUE) {
                DestroyWindow(hWnd);
                if (mode == CUSTOM_HOTKEY_MODE) {
                    UnregisterHotKey(NULL, REFRESH_CHANGE_HOTKEY);
                }
            }
            return 0;
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Initialize the Invisible Window
int initWindow(HINSTANCE hInstance) {
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Auto Refresh Change Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    hWnd = CreateWindowEx(
        0,  // Optional window styles.  // Previous Value: WS_EX_NOREDIRECTIONBITMAP
        CLASS_NAME,                 // Window class
        NULL,                       // Window text
        WS_DISABLED,                // Window style
        0, 0, 0, 0,                 // Size and position
        NULL,   // Parent window
        NULL,   // Menuk
        hInstance,   // Instance handle
        NULL    // Additional application data
        );

    if (hWnd == NULL) {
        return -1;
    }

    if (IsWindowVisible(hWnd) != 0) {
        cerr << "The \"Invisible\" Window is Visible... Aborting" << endl;
        return -1;
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    LONG result = 0;

    // Initialize the Invisible Window();
    result = initWindow(hInstance);

    if (result != 0) {
        cerr << "Failed to Initialize Window" << endl;
        return -1;
    }
    
    // Initialization for Mode Selection (if required)
    switch (mode) {
    case LENOVO_FN_REFRESH_FIX:
        // No custom initialization required...
        break;
    case CUSTOM_HOTKEY_MODE:
        if (!RegisterHotKey(NULL, REFRESH_CHANGE_HOTKEY, hotkeyMod, hotkeyVk)) {
            cerr << "Hotkey Registration Failed: Try Another Hotkey Combination" << endl;
            return -1;
        }
        break;
    default:
        cerr << "Not A Valid Mode Selected..." << endl;
        return -1;
        break;
    }
    
    // Get Initial Display Configs and Select the Display Path Index.
    result = getDisplayConfigs();
    if (result != ERROR_SUCCESS) {
        cerr << "Error Getting Display Configs: " << getDisplayConfigError(result) << endl;
        return -1;
    }
    result = selectDisplayPathIndex();
    if (result != 0) {
        return -1;
    }

    // Initialize and Set Refresh Rate Tracking Variable
    currentRefreshRate = getDisplayConfigRefresh();

    // Main Event Loop
    //  Handles Custom Hotkey Message
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        // Handle Hotkey Message
        if (mode == CUSTOM_HOTKEY_MODE && msg.message == WM_HOTKEY && msg.wParam == REFRESH_CHANGE_HOTKEY) {
            customHotkeyFixDisplayUpdate();
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
        continue;
    }
    
    return 0;
}