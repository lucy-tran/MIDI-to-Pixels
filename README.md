# MIDI to Pixels - Method 1 of the Music On Canvas Project

Download the midifile library from [here](https://midifile.sapp.org/) and put the folder at the root of the project. 

This method is run on the mscs1 machine. Please contact my advisor, Libby Shoop to set up an account and run the code via ssh.

## Project Structure

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
    ├── musicViz_omp3.cpp: **official* implementation of the parallel version, using parallel loop equal chunks
    ├── musicViz_seq2.cpp: *official* implementation of the sequential version
    ├── ...
   
   
 ## How to run
 
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
