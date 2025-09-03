Printing:
    CurrentLogLevel: Debug
    Palette:
        Enabled: false
    FileLogging:
        Enabled: true
        HClogPath: devlog_debug.hclog

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
                - arm64
            OperatingSystems:
                - any

    Build:
        Version: 0.0.0
        Channel: Developing # Developing/Unstable/Beta/Production
