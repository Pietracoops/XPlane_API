# XPlane API C++ Client
<p align="left">
    <a href="https://github.com/Pietracoops/XPlane_API/releases" target="_blank" rel="noopener"><img src="https://img.shields.io/github/release/Pietracoops/XPlane_API.svg" alt="Latest releases" /></a>
</p>

<p align="left">
	<a target="_blank" rel="noopener"><img src="https://img.shields.io/badge/Visual%20Studio-5C2D91.svg?style=for-the-badge&logo=visual-studio&logoColor=white" /></a>
	<a target="_blank" rel="noopener"><img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" /></a>
	<!--<a target="_blank" rel="noopener"><img src="https://img.shields.io/github/license/Ileriayo/markdown-badges?style=for-the-badge" /></a>-->
</p>

This repository contains a C++ UDP client that can be connected through python to send commands directly to XPlane. The Python Client can be accessed [at this link (XplaneApi)](https://pypi.org/project/XPlaneApi/). The github for the Python Client can be found [at this link (XplaneApi Github)](https://github.com/Pietracoops/XPlane_Python_Client). There is a demo script at the bottom of the documentation that can be used to speak directly to the C++ UDP Client (XPlane Server).

Open tasks for the API are tracked using [Jira](https://pilotai-api.atlassian.net/jira/software/c/projects/PIL/boards/1/backlog?view=detail&selectedIssue=PIL-21&epics=visible&issueLimit=100).

## Setup

This is a C++ application using and will thus require [Visual Studio Redistributables](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) for Visual Studio 2019. Essential runtimes have been packaged with binaries and can be retrieved on [Github](https://github.com/Pietracoops/XPlane_API/releases).

Install XPlane from here: https://drive.google.com/drive/folders/1stWGUvGKy5thNxMQn4vox1m7GDdMjS-7?usp=sharing

## Running the Setup

Follow these guidelines for successfull usage of the server

* Start XPlane.
* Wait until a flight has been started/resumed and you are in the aircraft.
* Navigate to the XPlanePackage_1.0.0/cpp_xp_server folder
	* Modify the subscriptions.txt to add or remove datarefs that you require subscription.
	* Execute the libXplane-udp-client.exe when ready
	* You can read the output to validate that the subscriptions have been successfully made.
* Follow the instructions in the Python Client repository to launch a client to communicate with this server.

## Architecture

Coming soon...