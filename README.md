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

**Hello World example** (for versions below 0.1.2-experimental)**:**  

```cpp
#include "MFWork/include/MFWork.h"

int main(int argc, char* argv[]) {
    MF::InternalSettings::SettingsStack::Setup(
        true, // <- start timer
        false, // -< allow overrides
        true, // <- check critical files
        true, // <- whether to parse arguments or not (disables in-terminal overriding)
        true, // <- auto determine log level
        true, // <- validate session
        true, // <- log build channel
        true, // <- alert on unstable channel
        MF::Print::LogLevel::Debug, // <- default log level
        true, // <- file logging
        "mfwork_logs.hclog" // <- file logging path
    );
    if (!MF::Initializer::InitializeMFWork(argc, argv)) {
        return -1;
    }

    MF::Print::Out(MF::Print::LogLevel::Info, "Hello World!");
    return 0;
}
```

**Hello World Example** (for versions above 0.1.2-experimental)**:**

```cpp
#include "MFWork/include/MFWork.h"


int main(int argc, char* argv[]) {
    MF::InternalSettings::GlobalSettings.Setup({
        // initialization
        .Init = {
            .StartTimer = true,
            .AllowOverrides = true,
            .CriticalFiles = {},
            .CheckCriticalFiles = true,
            .ParseArguments = true,
            .AutoDetermineLogLevel = true,
            .ValidateSession = true,
            .LogBuildChannel = true,
            .AlertOnUnstableChannel = true
        },
        // printing
        .Print = {
            .CurrentLevel = MF::Print::LogLevel::Debug,
            .File = {
                .Enabled = true,
                .HClogPath = "mfwork_logs.hclog"
            }
        },
        // project info
        .Project = {
            .App = {
                .Name = "MFWork-Testing",
                .Author = "mmashez",
                .License = "None",
                .Support = {
                    .Architectures = {"x86_64", "arm64", "any"},
                    .OperatingSystems = {"linux", "windows", "any"}
                }
            },
            .Build = {
                .Version = "0.0.0",
                .Channel = "unstable"
            }
        }
    });

    if (!MF::Initializer::InitializeMFWork(argc, argv)) {
        return -1;
    }

    MF::Print::Out(MF::Print::LogLevel::Info, "Hello World!");
    return 0;
}
```

---

## HotConfig  

HotConfig is MFWork’s custom configuration format, inspired by YAML.
**⚠️ Note**: Usage of App.hc and Build.hc is replaced in versions above v0.1.2-experimental. No deprecated support.
HotConfig will still be used for other purposes.

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
        # - any
# Comments are supported (both block and inline).
```

**Example: `Build.hc`** *(found in `Internal/Configurations/Rulebook/Build.hc`)*  

```yaml
version: 0.0.0
channel: Developing # Developing / Unstable / Beta / Production
```

Have fun! This project is yet to be improved.
