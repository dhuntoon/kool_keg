# Kool Keg
Purdue University > ECE Senior Design (ECE 49022) > Lab Section 5 > Team 51

This repository holds relevant files for Team 51 (_Kool Keg_) in ECE Senior Design. It includes hardware schematics, PCB layouts, mobile application software, and software diagrams.

## Table of Contents

kool_keg (repository home directory)

* `/Circuit Design/...`
  * system hardware schematics
 
* `/Kool Keg Circuit/...`
  * PCB Design and related files for system hardware

* `/Software/...`
  * Software subsystem working directory
  * `/aws_backend/...`
    * sub-directory contianing AWS Lambda code . 
    * This code is mirrored manually for reference and is **not** executed directly from this source.
    * **This directory holds code written by Dylan Huntoon for the subsystem**
  * `/figures/`...
    * sub-directory containing software flow diagrams and subsystem block diagram(s)
    * **This directory holds figures created by Dylan Huntoon for the subsystem**
  * `/keg_app/...`
    * sub-directory containing code related to the mobile app (Flutter SDK and dependencies)
    * `/lib/...`
      * sub-directory containing Flutter (Dart) code for the mobile app
      * **This directory holds code written by Dylan Huntoon for the subsystem**
  * `/testing_scripts/...` 
    * sub-directory containing Python code and JSON output for testing the software_subsystem
    * **This directory holds code written by Dylan Huntoon for the subsystem**
 
* `/README.md`
  * this markdown file

## Software Design

### Subsystem Block Diagram
The following diagram illustrates the components of the software subsystem and related interactions with other subsystems. The color-coding corresponds to the overall system diagram, found in the design documentation.

![Block Diagram](/Software/figures/Software-Block.png)

### Subsystem Software Architecture
The following diagram illustrates the software architecture for the system's mobile application. The program is written in Flutter (Dart) and is deployable on iOS and Android platforms. The black circle identifies the program entry point and is triggered when the application launches. 

![Block Diagram](/Software/figures/Software%20Structure.png)

For further detail, see the related figures in the `Software/figures/` subdirectory.

## Operating the Mobile App

Updated Nov 1, 2022.

The mobile application will be deployed onto devices supporting the iOS and Android platforms. Execution will automatically begin when the application is launched on the aforementioned devices.

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
$ cd kool_keg/Software/keg_app
$ flutter build target
```
where `target` is the targeted platform (ios, android, etc.)

To run the program:
```
$ cd kool_keg/Software/keg_app
$ flutter run lib/main.dart
```



## Credits

Contributors to this repository:
* Cole Kniffen (ckniffen@purdue.edu)
* Dylan Huntoon (dhuntoon@purdue.edu)
* Mitchell Morris (morri448@purdue.edu)
* Jared Nutt (nuttj@purdue.edu)
