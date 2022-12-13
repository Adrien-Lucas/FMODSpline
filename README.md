# FMODSpline

![Icon128](https://user-images.githubusercontent.com/16429096/207061172-1f93c66a-43ce-4018-85a1-0d512f7e51c1.png)

FMODSpline is a powerful tool for FMOD's Unreal Integration that adds a spline actor that can be used for creating immersive environments.

## Disclaimer

FMODSpline is not an official FMOD plugin! You need a license to use FMOD in your project. 
This plugins only extend the FMODStudio plugin to add functionalities for FMOD users.
No source code of FMOD or FMODStudio is contained within this plugin. 

## Install notes

* Clone plugin in your project's Plugins/ folder. 
* Regenerate visual studio project.
* Build your project using Visual Studio or Rider.
* Make sure that FMODStudio is enabled
![image](https://user-images.githubusercontent.com/16429096/207061814-bf0f7310-fde2-43e6-b7b7-bb338295a059.png)
* Enable the plugin in the Plugin tab 
![image](https://user-images.githubusercontent.com/16429096/207061638-8a7634da-1d52-487d-b018-be01150a798d.png)
* Restart Unreal

## How to

You can create a FMOD Spline by 
* (OR) Searching it in the "Place Actors" tab and drag it in the scene

  ![image](https://user-images.githubusercontent.com/16429096/207062397-42ba8bea-e6f2-4caf-8c0c-a9f364d93848.png)
* (OR) Show Plugins content in you Content Browser, go to Plugins > FMOD Spline Content > FMOD Spline C++ Classes > FMODSpline and drag it in the scne
* (OR) Create a blueprint that inherits from FMODSpline

### Simple Audio Spline

![SimpleAudioSpline](https://user-images.githubusercontent.com/16429096/207062885-fd1d343c-ebe2-453d-99a4-a17a02227fb5.png)

When making a non closed spline, the FMOD Event will try to be the closest as possible from the listener on the Spline

#### Parameters

![image](https://user-images.githubusercontent.com/16429096/207065702-0e3a6f38-eb53-4801-b309-e36c59ba4e1c.png)

[FMODEvent] is the fmod event you want to play along the spline. It should be 3D.

### Simple Audio Volume

![Simple Audio Volume](https://user-images.githubusercontent.com/16429096/207062883-ca603f7f-4c1c-4755-82c1-446d8bc71ed3.png)

When making a closed spline, the FMOD Event will be at the closest point on the spline from the listener when they are outside, but will stick to the listener's position when they are inside of the shape defined by the spline, simulating a 2D sound.

#### Parameters

![image](https://user-images.githubusercontent.com/16429096/207066074-30443df5-dd9a-40dc-8524-6705a4fbe1f0.png)

[FMODReverbEvent] The reverb event you want to play when entering the volume.

[FMODMixEvent] The mix event you want to play when entering the volume.

[Cylinder height] The height limit of the volume, if the listener is not in the cylinder, they wont have the "2D Sound simulation".

### Openings

![Openings](https://user-images.githubusercontent.com/16429096/207062873-4e6f32e7-d0d5-4ce8-8938-8fa1d2addab9.png)

You can chose for you spline to allow only for some part of it to emit sound when outside of it. It simulates a "door" effect, letting the sound from the inside of the volume to leave it only at the openings defined by the designer.

A door can be closed by changing it's default bOpen parameter, or calling SetOpeningState() at runtime on it. Closing or opening a door changes the value of the "Portal_IsOpen" fmod parameter. This allows for sound designers to make the sound different if the door is closed or simply put it's volume to 0.

Note : Openings works for simple audio splines too

#### Parameters

![image](https://user-images.githubusercontent.com/16429096/207066780-a6dbd152-689a-4c2c-967e-92aa60299abc.png)

[Openings] The array of all the openings. 

	Spline Point is the spline point where the opening will be centered. 
	
	Width is the width in cm of the opening. Open decides if this opening is opened by default.

### Random Sounds

![RandomSounds](https://user-images.githubusercontent.com/16429096/207062880-22623b20-c424-4615-9512-09a9a5aabf2f.png)

Random sounds will play at the random intervals you gave, inside of the volume. If possible, they will avoid to spawn too close from the listener.

### Parameters 

![image](https://user-images.githubusercontent.com/16429096/207067300-9c068485-3c8c-4576-b60c-8942681c5442.png)

[RandomSounds] The array of all the random sounds that will play inside of the volume. 

	Event is the fmod event of the random sound. 
	
	Delay range defines MinDelay and MaxDelay between two similar spawns of this sound.
	
[MinRandomSoundDistance] The safe distance random sounds will try to respect when spawning near from the listener. This prevents the listener from noticing a very close spatialized sound where there is nothing in the game space.

### Global Parameters
![image](https://user-images.githubusercontent.com/16429096/207068139-db8b6623-ac85-4f23-bc97-c70121453238.png)

[MaxDistance] the distance at which the FMODSpline completly shuts down

![image](https://user-images.githubusercontent.com/16429096/207068331-5073b02f-a851-4790-abf8-df25e67d574b.png)

[bShowDebug] Will show the max distance radius, the volume cylinder, and the position of the audio component at runtime.

[OpeningDebugPrecision] The precision of the opening lines debug visualization.

#### Note

This plugin was wrote with the help of GitHub Copilot and ChatGPT. Check out this really impressive tools!


