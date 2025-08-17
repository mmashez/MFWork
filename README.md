# MFWork  
**Build Faster, Ship Safer.**

---

## What is MFWork?  

MFWork is a framework designed to make building applications **faster and more secure**.  
Currently, it provides only basic features such as:  

- A custom configuration system (HotConfig)  
- A file manager  
- Session validation  

⚠️ The initial build (**0.0.0, 17/08/25**) is unstable. Please report any issues you encounter while using the framework.  

---

## Supported Platforms  

MFWork’s code is not strictly tied to a specific platform or CPU architecture,  
but you may still run into issues.  

- **Officially supported**: Linux (x86_64)  
- **Note**: GUI support using GTK is **not guaranteed**, as it is distribution-dependent.  

---

## Compiling an Application With MFWork  

If you’re on Linux, use `pkg-config` to avoid problems with third-party headers.  

**clang++ example:**  
```bash
clang++ target.cpp -o output `pkg-config gtkmm-4.0 --cflags --libs`
```

**g++ example:**  
```bash
g++ target.cpp -o output `pkg-config gtkmm-4.0 --cflags --libs`
```

*(The above examples add include folders and libraries for the gtkmm-4.0 header,  
as used in `Internal/GUI/Foundation/Base.hpp`.)*  

---

## Example Usage  

**Minimal Example:**  
```cpp
#include "MFWork/include/MFWork.h"

int main() {
    MF::Initializer::InitializeMFWork(); // checks for mandatory files and validates session

    std::string UnusedBuffer = "";
    MF::FilesManager::CreateDirectory("TestDirectory", UnusedBuffer);
    if (MF::FilesManager::Exists("TestDirectory")) {
        MF::Print::Out(MF::Print::LogLevel::Info, "Directory created successfully!");
    }
    return 0;
}
```

**Or if you want error handling:**  
```cpp
#include "MFWork/include/MFWork.h"
#include <string>

int main() {
    MF::Initializer::InitializeMFWork();
    std::string FileManagerFault;

    MF::FilesManager::CreateDirectory("TestDirectory", FileManagerFault);
    if (!FileManagerFault.empty()) {
        MF::Print::Out(MF::Print::LogLevel::Error, "Something went wrong: " + FileManagerFault);
        return 1;
    }

    if (MF::FilesManager::Exists("TestDirectory")) {
        MF::Print::Out(MF::Print::LogLevel::Info, "Directory created successfully!");
    }
    return 0;
}
```

---

## HotConfig  

HotConfig is MFWork’s custom configuration format, inspired by YAML.  

**Example: `App.hc`** *(found in `Internal/Configurations/Rulebook/App.hc`)*  
```yaml
app: MFWork-Testing # Must not contain spaces
author: mmashez
license: None
support:
    Architecture: 
        - x86_64
        - arm64
        # - arm
        # - x86
    OS:
        - linux
        # - windows
        # - any    << not implemented yet
# Comments are supported (both block and inline).
```

**Example: `Build.hc`** *(found in `Internal/Configurations/Rulebook/Build.hc`)*  
```yaml
version: 0.0.0
channel: Developing # Developing / Unstable / Beta / Production
```

Have fun! This project is yet to be improved.
