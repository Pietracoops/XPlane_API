# Installation of Josh API

These are the installation instructions that have been tested and proven functional on Windows 10 Platform

*NOTE:* If any of the links to the websites below do not work, I have uploaded all the files you need to my google drive, you can download them [Here](https://drive.google.com/drive/folders/1stWGUvGKy5thNxMQn4vox1m7GDdMjS-7?usp=sharing) 

## Installing X-Plane

Fetch the X-Plane file in the google drive link above and follow readme instructions within the zip.

## Setting up the API
* Download Python 2.7.9 and add it to your path - Xplane will search for the python27.dll file upon bootup so make sure to add it to your path (note, 2.7.10 will not work and crash xplane)
* Download the [Python interface](http://www.xpluginsdk.org/python_interface_latest_downloads.htm) for X-Plane and place it in the resources/plugins folder. If starting coding with this interface, also highly recommended to get the example scripts from these links:
	* [PythonScripts.zip](http://www.xpluginsdk.org/python_interface_downloads.htm) - OPTIONAL
* It is also recommended to get the [Dataref Editor](https://www.xsquawkbox.net/xpsdk/mediawiki/DataRefEditor) (OPTIONAL):
* Unzip the zipped folders Locate your copy of X-Plane 11, then go to the Resources/plugins folder, and move the two folders there
* In order to run the python plugins we need to install a couple of packages for Python 2.7 (further details can be seen in requirements):

```
	-pip install --upgrade setuptools==44.1.1
	-pip install --upgrade "pip < 21.0"
	-pip install pillow
	-pip install Enum
	-pip install PyAutoGUI
	-pip install PyAudio
```