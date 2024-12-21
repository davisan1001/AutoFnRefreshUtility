# autoFnRefreshChange

This application is an automatic monitor refresh changer on hotkey press.  
It supports two modes:
  1. Lenovo Fn+R Refresh Fix.
  2. Custom Hotkey Refresh Change.

#### Lenovo Fn+R Refresh Fix
The application was built with the intent to fix the Lenovo Legion Fn+R hotkey:  
  -  **The Problem:** The Legion Fn+R hotkey would successfully change the monitor refresh rate from 60 to 165Hz and back, BUT when going from 165 to 60Hz the active signal mode would remain at 165Hz. This would not allow for the benefits of battery life to be gained.  
  -  **The Fix:** This program detects an Fn+R key press and fixes the active signal mode to reflect the monitor update.  

#### Custom Hotkey Refresh Change
The application has been extended to include a mode to configure your own refresh change hotkey (for non-Lenovo Legion laptops or those that do not support the Fn+R hotkey).  
The hotkey can be configured using the global variables at the top of the program. Custom variables are marked with the `// USER: ...` comment. 

## Getting Started

### Dependencies

- Windows SDK
- C++
- cl.exe Windows C/C++ compiler

### Installing

- Clone the repository to your local machine.
- Compile the autoFnRefreshChange.cpp file using your favorite C++ compiler for windows.
  - If using cl.exe, run: `cl.exe /EHsc /nologo /Fe.\autoFnRefreshChange.exe autoFnRefreshChange.cpp user32.lib`
- **... TODO: Add complete instructions for installing**

### Executing

Once you obtain the executable for this program, you have two options to run the program on startup.
1. Create a shortcut to the .exe and place the shortcut in: `C:\Users\<user>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\`
2. Place the .exe in the `C:\Users\<user>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\` folder directly. **NOTE: This can cause Windows Defender to view the program as malware**.    


## For the Developer

### VS Code Compilation
When compiling from VS Code, ensure that you have a .vscode sub-directory in the project directory with:
- A tasks.json file with the following setup:
  ```
  {
      "tasks": [
          {
              "type": "cppbuild",
              "label": "C/C++: cl.exe build active file",
              "command": "cl.exe",
              "args": [
                  //"/Zi", // Uncomment to enable output debugging intermediary files (.ilk, .pdb)
                  "/EHsc",
                  "/nologo",
                  "/Fo${fileDirname}\\build\\",
                  "/Fe${fileDirname}\\build\\${fileBasenameNoExtension}.exe",
                  "${file}",
                  "user32.lib"
              ],
              "options": {
                  "cwd": "${fileDirname}"
              },
              "problemMatcher": [
                  "$msCompile"
              ],
              "group": {
                  "kind": "build",
                  "isDefault": true
              },
              "detail": "Build Task."
          }
      ],
      "version": "2.0.0"
  }
  ```
- A settings.json file with the following setup: **TODO: I honestly don't know if this is necessary**
  ```
  {
      "files.associations": {
          "ostream": "cpp",
          "vector": "cpp",
          "iostream": "cpp"
      }
  }
  ```
