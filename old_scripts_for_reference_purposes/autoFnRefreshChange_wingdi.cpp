#define UNICODE

#include <Windows.h>
#include <iostream>
#include <vector>

#include <cfgmgr32.h>


using namespace std;

const WCHAR* monitorDevice = L"Lenovo DisplayHDR"; // Monitor name for lenovo integrated display
PDISPLAY_DEVICE adapterDev;
PDISPLAY_DEVICE monitorDev;


// List all graphics devices
int getAllGraphicsDevices() {
    DISPLAY_DEVICEW dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(DISPLAY_DEVICEW);


    DWORD adapterNum = 0;
    while (0 != EnumDisplayDevices(NULL, adapterNum, &dd, 0)) {
        wcout << dd.DeviceName << "\t" << dd.DeviceString << endl;
        wcout << "deviceFlags:\t" << dd.StateFlags << endl;

        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            cout << "PRIMARY\t";
        }
        if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            cout << "ACTIVE\t";
        }
        cout << endl;
        

        DISPLAY_DEVICE monitordd = {0};  
        monitordd.cb = sizeof(DISPLAY_DEVICE);

        DWORD monitorNum = 0;
        while (0 != EnumDisplayDevices(dd.DeviceName, monitorNum, &monitordd, 0)) {
            wcout << monitordd.DeviceName << "\t" << monitordd.DeviceID << "\t" << monitordd.DeviceString << endl;
            monitorNum++;
        }
        
        adapterNum++;
    }
    cout << endl;

    return 0;
}

// Get device attached to specific display/monitor
int getGraphicsDeviceForDisplay(const WCHAR* monitorName) {
    DISPLAY_DEVICEW dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(DISPLAY_DEVICEW);


    DWORD adapterNum = 0;
    while (0 != EnumDisplayDevices(NULL, adapterNum, &dd, 0)) {
        if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE) { // An extra (probably unnecessary) check to make sure that the display device is active
            DISPLAY_DEVICE monitordd = {0};  
            monitordd.cb = sizeof(DISPLAY_DEVICE);

            DWORD monitorNum = 0;
            while (0 != EnumDisplayDevices(dd.DeviceName, monitorNum, &monitordd, 0)) {
                if((wstring)monitordd.DeviceString == (wstring)monitorName) { // Casting to wstring to use the == operator
                    memcpy(monitorDev, &monitordd, sizeof(DISPLAY_DEVICE));
                    memcpy(adapterDev, &dd, sizeof(DISPLAY_DEVICE));
                    return 0;
                }
                monitorNum++;
            }
        }
        
        adapterNum++;
    }

    return -1;
}

// Prints important display properties
int printDisplayProperties(DEVMODE* dm) {
    if (0 != EnumDisplaySettings(adapterDev->DeviceName, ENUM_CURRENT_SETTINGS, dm)) {
        cout << "Current Display Frequency = " << dm->dmDisplayFrequency << endl;
        cout << "Current Display Resolution = " << dm->dmPelsWidth << " x " << dm->dmPelsHeight << endl;
        cout << "Current Display BitsPerPixel = " << dm->dmBitsPerPel << endl;
        cout << "Current Display Fixed Output = " << dm->dmDisplayFixedOutput << endl;
        cout << "Current Display Orientation = " << dm->dmDisplayOrientation << endl;
        cout << "Current Display Flags = " << dm->dmDisplayFlags << endl;
        cout << dm->dmDriverVersion << "\t" << dm->dmDriverExtra << endl;
    }
    return 0;
}

// Sets display properties (currently only supports frequency)
int setDisplayProperties(DEVMODE* dm, DWORD newFreq) {
    if (0 != EnumDisplaySettings(adapterDev->DeviceName, ENUM_CURRENT_SETTINGS, dm)) {
        cout << "Current Display Frequency = " << dm->dmDisplayFrequency << endl;

        dm->dmDisplayFrequency = newFreq; //set the DisplayFrequency
        //dm.dmPelsWidth = 1280;
        //dm.dmPelsHeight = 800;
        LONG ret = ChangeDisplaySettingsEx(NULL, dm, NULL, 0, NULL); 
        std::cout << "ChangeDisplaySettingsEx returned " << ret << '\n';
        if (0 != EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, dm)) {
            cout << "DisplayFrequency after setting = " << dm->dmDisplayFrequency << endl;
        }
        switch (ret) {
        case DISP_CHANGE_SUCCESSFUL: 
            std::cout << "display successfully changed\n";
            return 0;
            break;
        case DISP_CHANGE_BADDUALVIEW:
            std::cout << "The settings change was unsuccessful because the system is DualView capable\n";
            break;
        case DISP_CHANGE_BADFLAGS: 
            std::cout << "An invalid set of flags was passed in.\n"; 
            break;
        case DISP_CHANGE_BADMODE: 
            std::cout << "The graphics mode is not supported.\n"; 
            break;
        case DISP_CHANGE_BADPARAM: 
            std::cout << "An invalid parameter was passed in. This can include an invalid flag or combination of flags.\n"; 
            break;
        case DISP_CHANGE_FAILED: 
            std::cout << "The display driver failed to change to the specified graphics mode.\n"; 
            break;
        case DISP_CHANGE_NOTUPDATED: 
            std::cout << "Unable to write settings to the registry.\n"; 
            break;
        case DISP_CHANGE_RESTART: 
            std::cout << "The computer must be restarted for the graphics mode to work.\n";
            break;
        }
    }
    return -1;
}


int main(int argc, const char* argv[]) {
    adapterDev = (PDISPLAY_DEVICE)malloc(sizeof(DISPLAY_DEVICE));
    adapterDev->cb = sizeof(DISPLAY_DEVICE);

    monitorDev = (PDISPLAY_DEVICE)malloc(sizeof(DISPLAY_DEVICE));
    monitorDev->cb = sizeof(DISPLAY_DEVICE);

    DEVMODE adapterDM;
    ZeroMemory(&adapterDM, sizeof(adapterDM));
    adapterDM.dmSize = sizeof(adapterDM);

    // TODO: Unused
    DEVMODE monitorDM;
    ZeroMemory(&monitorDM, sizeof(monitorDM));
    monitorDM.dmSize = sizeof(monitorDM);

    // Get list of all connected display devices
    getAllGraphicsDevices();

    // Get device attached to Lenovo integrated display.
    if (0 != getGraphicsDeviceForDisplay(monitorDevice)) {
        cerr << "Failed to Find Device Attached to Lenovo Integrated Display Device" << endl;
        return -1;
    }
    
    printDisplayProperties(&adapterDM);

    setDisplayProperties(&adapterDM, 165);

    // TODO: Copy the current devmode to defaultDevMode

    // TODO: Copy the current devmode to newDevMode
    // TODO: Modify newDevMode and set it with changeDisplaySettingsEx
    //      Try setting resolution temporarily to 1280 x 800 and then back to 2560 x 1600
}