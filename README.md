# MFWork  

**Build Faster, Ship Safer.**

---

## What is MFWork?  

MFWork is a framework designed to make building applications **faster and more secure**.  
Currently, it provides only basic features such as:  

- A custom configuration system (HotConfig)  
- A file manager  
- Session validation  

⚠️ **Note:**
The initial build (**0.0.0, 17/08/25**) is unstable. Please report any issues you encounter while using the framework.
Breaking API changes will keep happening until MFWork builds a stable foundation, which is planned for around version 1.0.0.

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

Or, for versions **above 0.1.2-experimental**:

Using Project.hc:
```cpp
#include "MFWork/include/MFWork.h"

int main(int argc, char* argv[]) {
    MF::InternalSettings::SetupHC("Project.hc");

    if (!MF::Initializer::InitializeMFWork(argc, argv)) {
        return -1;
    }

    MF::Print::Out(MF::Print::LogLevel::Info, "Hello World!");
    return 0;
}
```
(Your Project.hc is supposed to look something like:

```yaml
Printing:
CurrentLogLevel: Debug
Palette:
    Enabled: true
    
InitializationSettings:
    StartTimer: true
    AllowOverrides: false
    ParseArguments: true
    CheckCriticalFiles: true
    AutoDetermineLogLevel: false
    ValidateSession: true
    LogBuildChannel: true
    AlertOnUnstableChannel: true
    CriticalFiles:
        - None
    
Project:
    App:
        AppName: Meow
        Author: mmashez
        License: None
        Support:
            Architectures:
                - x86_64
                # arm64
                # x86
                # any
            OperatingSystems:
                - any
                # linux
                # windows
    Build:
        Version: 0.0.0
        Channel: Developing # Developing/Unstable/Beta/Production
```
)

Or, using manual setup:

```cpp
#include "MFWork/include/MFWork.h"

int main(int argc, char* argv[]) {
    MF::InternalSettings::SettingsStack settings = {
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
        .Print = {
            .CurrentLevel = MF::Print::LogLevel::Debug,
            .File = {
                .Enabled = true,
                .HClogPath = "mfwork_logs.hclog"
            },
            .Colors = {
                .Enabled = false
            }
        },
        .Project = {
            .App = {
                .Name = "MFWork-Testing",
                .Support = {
                    .Architectures = {"any"},
                    .OperatingSystems = {"any"}
                }
            },
            .Build = {
                .Version = "0.0.0-dev",
                .Channel = "Developing"
            }
        }
    };
    MF::InternalSettings::Setup(&settings);

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

**Example: `Project.hc`** *(found in `Internal/Configurations/Rulebook/App.hc`)*  

```yaml
Printing:
CurrentLogLevel: Debug
Palette:
    Enabled: true
    
InitializationSettings:
    StartTimer: true
    AllowOverrides: false
    ParseArguments: true
    CheckCriticalFiles: true
    AutoDetermineLogLevel: false
    ValidateSession: true
    LogBuildChannel: true
    AlertOnUnstableChannel: true
    CriticalFiles:
        - None
    
Project:
    App:
        AppName: Meow
        Author: mmashez
        License: None
        Support:
            Architectures:
                - x86_64
                # arm64
                # x86
                # any
            OperatingSystems:
                - any
                # linux
                # windows
    Build:
        Version: 0.0.0
        Channel: Developing # Developing/Unstable/Beta/Production # Comments can be defined like this,
# or like this.
```

Have fun! This project is yet to be improved.
