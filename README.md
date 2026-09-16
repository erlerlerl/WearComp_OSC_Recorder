# WearComp OSC Collection

This Repository includes the code for a custom OSC Recording and Playback Software as well as some testrecordings done with sensors build by the [WearableComputingGroup](https://github.com/wearablecomputing) at University of the Arts Berlin, as part of the [Technologies of Touch](https://www.udk-berlin.de/en/courses/fashion/research/wearable-computing/projects/technologies-of-touch/) project.

## WearComp_OSC_Recorder

Still under construction

## Testrecordings and Max Visualizer

In the [testrecordings](/testrecordings) folder there are three recordings of OSC Sensor data. The recording tool, used sensors and OSC address taxonomy are as explained below. Out of the three recordings [record3.osc](/testrecordings/record3.osc) represents the sensor data the best.
[This repository](https://github.com/basilstotz/oscrecorder) was used to record the osc data and should be used to play it back. To install it follow the instructions in [here](https://github.com/basilstotz/oscrecorder/blob/main/README.md). To play back any of the testrecordings use the following command and replace _filename_ with the file you want to play back, _ip_ with the ip address of your listener and _port_ with the respective port.

``` 
oscplay <filename>.osc | oscemit / <ip>:<port>
```

### Sensor data and OSC address taxonomy

Two types of sensors were recorded. The sensors were connected to a [Bela Gem Multi](https://bela.io/products/bela-gem-stereo-and-multi/) board which acts as a OSC sender. The code running on the Bela Board can be found [here](/BelaPatches/CoscSender/render.cpp).
One sensor is a piezo-electric sensor, responding to manual triggering and calculating a velocity value for each trigger. The piezo-element uses the Belas Audio Input on Channel 0 and is sampled on audio rate. The OSC messaging only occurs on trigger.
The other sensor is a resisistive pressure sensor. It sends every 10 analogFrames. The exact timing is determined by the Bela Hardware and the used sampleRate. For the testrecordings following settings were used:

``` 
    TODO add correct values
```

Sending every 10 analogFrames was a decision to achieve a sufficient temporal resolution of a continuous signal while reducing the amount of OSC traffic in the network and minimizing the risk of packet loss.

#### OSC address taxonomy

The piezo-electric value is sent via the address `/bela/piezo/peak`. `/bela` marking the Bela Gem Multi as the origin of the sensor value. `/piezo` marking the type of sensor used and `/peak` marking the type of value retrieved from the sensor. In this case it resembles detected signal peaks. In future development something like a `/bela/piezo/raw` address could be used to send the raw piezo data in audio rate through the network (via [AOO](https://git.iem.at/aoo/aoo)).

The resistive pressure sensor is specified by the `/analog` sensor type and `/resistive` as value type resulting in an OSC address of `/bela/analog/resistive`. 

### Signal Characterstics

The piezo electric OSC Message delivers 2 values. As the Code on the Bela Gem Multi support Multichannel Peakdetection the first value represents the channel on which the peak was detected as an `int`. The second value represents the calculated velocity as a `float` value, ranging from 0.0 to 1.0. As described [above](#Sensor-Data-and-OSC-address-taxonomy) the messaging only occurs on a detected peak, so __the signal is event-based and not continous___.

The resistive pressure sensor delivers only the raw values received from reading the analog Input. The value is also sent as a `float` ranging from 0.0 to 1.0. __This signal is meant to be treated as a continuous signal__.

### Current Limitations

Especially for the continuous messaging of the resistive pressure sensor values there are a few limitations with the current setup. Right now, the values of the sensor are not sent as blocks, but every value gets its own message. This fast paced communication seems to push the used osc recording tool to its limits, as there are visible gaps in the OSC data, when rendered in the [OSC Visualizer](/MaxPatches/OSCVisualizer.maxpat). It is probably best to interpolate the values retrieved from this address to achieve a smooth response when processing the values.

There are some other timing imprecisions (Bela analog framerate, windowing while peak detection) which will be left aside here and probably won't matter for the specific usecase.


