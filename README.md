# Kool Keg
Purdue University > ECE Senior Design (ECE 49022) > Lab Section 5 > Team 51

This repository holds relevant files for Team 51 (_Kool Keg_) in ECE Senior Design. It includes hardware schematics, PCB layouts, mobile application software, and software diagrams.

## Table of Contents

kool_keg (repository home directory)
* `/Circuit Design/...`
  * system hardware schematics
  * circuit layouts for PCB Design
  
* `/Software Design/...`
  * software flow diagrams
  * sub-system block diagram(s)

* `/keg-app/...`
  * mobile application working directory
  * includes the standard SDK with necessary libraries for cross-platform support (see `/keg_app/android/` and `/keg_app/ios/`)
  * `/keg_app/lib/...`
    * sub-directory containing Flutter (Dart) code for the mobile app
 
* `/README.md`
  * this markdown file

## Software Design

### Subsystem Block Diagram
The following diagram illustrates the components of the software subsystem and related interactions with other subsystems. The color-coding corresponds to the overall system diagram, found in the design documentation.

![Block Diagram](/Software_Design/Software-Block.png)

### Subsystem Software Architecture
The following diagram illustrates the software architecture for the system's mobile application. The program is written in Flutter (Dart) and is deployable on iOS and Android platforms. The black circle identifies the program entry point and is triggered when the application launches. 

![Block Diagram](/Software_Design/Software%20Structure.png)

For further detail, see the related `Data Management` and `Object Structure` figures in the `Software_Design` subdirectory.

## Operating the Mobile App

Updated Sep 27, 2022.

The mobile application will be deployed onto devices supporting the iOS and Android platforms. Execution will automatically begin when the application is launched on the aforementioned device.

To build or prototype with the program on a local machine, you must install the following dependencies. To ensure dependencies are installed correctly, reference the flutter documentation (https://docs.flutter.dev/get-started/install).

Dependencies:
* Flutter (Flutter SDK 3.3.1)
* Android toolchain (Android SDK version 33.0.0)
* Xcode (Xcode 14.0) - for building and emulating iOS applications
* Android Studio (version 2021.2) - for emulating Android applications

For rapid prototyping and testing, an IDE (such as VSCode or XCode) is recommended. Alternative command line operations are listed below for Unix terminals. 

### Command Line Operations

See the following documentation for Flutter Command Line Tools: https://docs.flutter.dev/reference/flutter-cli.

```$ flutter --version``` returns the version of flutter installed on your machine. 

```$ flutter doctor``` returns the dependency checks for the aforementioned libraries. Helpful installation directions are provided for missing dependencies. 

To build the program:
```
$ cd kool_keg/keg-app/keg_app
$ flutter build target
```
where `target` is the targeted platform (ios, android, etc.)

To run the program:
```
$ cd kool_keg/keg-app/keg_app
$ flutter run lib/main.dart
```



## Credits

Contributors to this repository:
* Cole Kniffen (ckniffen@purdue.edu)
* Dylan Huntoon (dhuntoon@purdue.edu)
* Mitchell Morris (morri448@purdue.edu)
* Jared Nutt (nuttj@purdue.edu)
