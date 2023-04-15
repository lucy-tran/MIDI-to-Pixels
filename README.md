# MIDI to Pixels - Method 1 of the Music On Canvas Project

Download the midifile library from [here](https://midifile.sapp.org/) and put the folder at the root of the project. 

This method is run on the mscs1 machine. Please contact my advisor, Libby Shoop to set up an account and run the code via ssh.

## Project Structure
```bash
.
├── Makefile
├── Xauthority
├── bin
│   ├── audio_samples
│   │   ├── ...
│   ├── draw2D.gnu
│   ├── midi_samples
│   │   ├── ...
│   ├── output_images
│   │   ├── ...
│   ├── rgb.dat
│   └── runtime_results
│       ├── ...
├── midifile: Library to process .midi files. Need to download separately from (here)[https://midifile.sapp.org/]
└── src
    ├── musicViz_omp2.2.cpp: parallel version using parallel loop chunks of 1
    ├── musicViz_omp3.cpp: OFFICIAL implementation of the parallel version, using parallel loop equal chunks
    ├── musicViz_seq2.cpp: OFFICIAL implementation of the sequential version
    ├── ...
 ```  
   
 ## How to run the code
 
 1. To generate an image for a piece of music, first put a .midi file into the `bin/midi_samples` folder. 
 
 2. Then, `cd` to the `bin` folder and run:
 
```
./musicViz_omp3 midi_samples/<the name of the .midi file>
```

to execute the parallel version. To run the sequential version, enter:

```
./musicViz_seq2 midi_samples/<the name of the .midi file>
```

This will generate a `rgb.dat` file in the `bin` folder. This file contains the x:y:R:G:B data for the image to be generated. 

3. To plot the image from this file, simply run:

```
gnuplot draw2D.gnu
```

In the `bin` folder, there is now an `output.png` file. This is the visualization of the music you input. Move this file to the `output_images` if you want.

To change the number of threads in the parallel version, go to `src/musicViz_omp3` and edit line 69.

```
int numThreads = 8; // change this number
```

Then, remake the executable by navigating to the root directory and running:

```
make musicViz_omp3
```

Go back to the `bin` folder and repeat steps 2 and 3.

## Pseudocode

```
Loop through the events before the first NOTE-ON event: //first for-loop
    If the event type is Control Change:
        Initialize the channel's width to 1
        Map the instrument number and channel's width to the channel

//Some MIDI files do not have Control Change events.
If the number of channel = 0 or an instrument is not set:
    Default the only channel to instrument 1, which is acoustic grand piano

//File duration (in ticks) can be very large, so I scale the file duration down to 500 to avoid running out of memory.
Scale <- File duration / 500
File duration <- 500

Continue looping through events: //second for-loop
    If the event type is NOTE-ON: // all non-NOTEON events are skipped
	     Get the channel where this event happens
        Starting x-coordinate <- Event's starting time/Scale
        Ending x-coordinate <- Starting x-coordinate + Event's duration/Scale
		
        Initialize y-coordinate to 0
        While ColorMap[channel][y-coordinate][starting x-coordinate] is already painted:
            y-coordinate++
            If y-coordinate == current channel's width:
                Increase channel's width by 1

        Convert sound/instrument number to hue (H)
        Convert velocity (volume) to saturation (S)
        Convert pitch to brightness (B)

        Convert HSB to RGB color, because GNU Plot needs RGB
        ColorMap[channel][y_coord][Starting x_coord:Ending x_coord] = RGB color value
```

Writing the data file in the x:y:R:G:B format:
```
Loop through each channel:
    Loop through the rows of the current channel, from 0 to channelWidth:
        Loop through the scaled duration of the whole file (x-axis):
            Append the x, y, R, G, B data of current cell into a file for GNU plot to draw from
```
