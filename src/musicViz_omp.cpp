#include "MidiFile.h"
#include "Options.h"
#include "utils.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <fstream>

#include <omp.h>

using namespace smf;
using namespace std;

float scaleDown(int fileDuration);
void prtRgb(int instrCount, int maxWidth, int numTicks, RGB *colorMap1, string fname);

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   MidiFile midifile;
   if (options.getArgCount() == 0) midifile.read(cin);
   else midifile.read(options.getArg(1));
   midifile.joinTracks(); // midifile.getTrackCount() will now return 1
   midifile.linkNotePairs();

   // MIDI variables
   MidiEvent* mev;
   map<int, vector<int>> channelMap; //mapping from channel idx to [sound number, channel width]
   map<int, int> channelIdx; // mapping from channel number to its index in colorMap
   int eventItr = 0;
   int numEvents = midifile[0].size();

   // Get the number of instruments and their channel designation
   while (eventItr < numEvents && !midifile[0][eventItr].isNoteOn()) {
      mev = &midifile[0][eventItr];
      if (mev->getCommandNibble() == 192) { //a program change event, which sets up a sound on a channel
         int channel = mev->getChannel();
         int instr = (int)(*mev)[1];
         vector<int> info = {instr, 1}; // channel width is initialized to 1
         channelMap.insert(pair<int, vector<int>>(channel, info));
         channelIdx.insert(pair<int, int>(channel, channelMap.size()-1));
         //Debugging print
         // cout << "Set channel " << channel << " to " << (int)(*mev)[1] << endl;
      }
      eventItr++;
   }

   if (channelMap.size() == 0) { // if there is no explicit designation of an instrument to a channel
      vector<int> info = {0, 1};
      channelMap.insert(pair<int, vector<int>>(0, info)); //default the only channel to 'acoustic grand piano'
   }

   int fileDurationInTicks = midifile.getFileDurationInTicks();
   cout << "Real file duration: " << fileDurationInTicks << endl;

   float scale = scaleDown(fileDurationInTicks);
   fileDurationInTicks /= scale;
   int maxWidth = 1;

   cout << "Instrument count: " << channelMap.size() << endl;
   cout << "File duration: " << fileDurationInTicks << endl;

   //OpenMP variables
   map<int, vector<MidiEvent>> channelEvents;
   // vector<MidiEvent> myEventList;
   RGB colorMap[channelMap.size()][10][fileDurationInTicks]; //different from the sequential version
   //set num thread
   //split the one track into channels of noteOn events, in parallel
   omp_set_num_threads(channelMap.size());
   int numThreads = omp_get_num_threads();

   double start = omp_get_wtime();

   #pragma omp parallel shared(numEvents, numThreads, eventItr, channelIdx, channelEvents, channelMap, midifile, colorMap, scale) \
   reduction(max:maxWidth) default(none) 
   {
      int id = omp_get_thread_num();
      //This for loop is essentially group the note-on events by their channels
      for (int event=eventItr+id; event<numEvents; event+=numThreads) { //parallel loop chunks of 1
         MidiEvent mev = midifile[0][event];
         if (mev.isNoteOn()) {
            int channel = mev.getChannel();
            int cIdx = channelIdx[channel];
            #pragma omp critical  
            {
               if (channelEvents.find(cIdx)==channelEvents.end()) {
                  vector<MidiEvent> events = {mev};
                  channelEvents[cIdx] = events;
               } else {
                  channelEvents[cIdx].push_back(mev);
               }
            }
         }
      }

      #pragma omp barrier 

      for (MidiEvent e: channelEvents[id]) {
         int xStart = (e.tick)/scale;
         int xEnd = xStart + (e.getTickDuration())/scale;
         int y = 0;
         while (y<9 && (colorMap[id][y][xStart].R!=0 || 
            colorMap[id][y][xStart].G!=0 || 
            colorMap[id][y][xStart].B!=0)) { //the starting cell is painted 
            y++;    //proceed to the next row
            if (y == channelMap[id][1]) {
               channelMap[id][1]++;
               maxWidth = max(channelMap[id][1], maxWidth);
            }
         }

         float hue = (float)channelMap[id][0] * 2.8125; // hue ∈ [0,359], instrument ∈ [0,127]
         float sat = ((float) e[2]) / 127.0 * 100.0; // saturation ∈ [0,100] , velocity (~volume) ∈ [0, 127]
         float val = ((float) e[1]) / 127.0 * 100.0; // value ∈ [0,100] , pitch ∈ [0, 127]
         RGB rgb = HSVtoRGB(hue, sat, val);

         for (int x=xStart; x<=xEnd; x++) {
            colorMap[id][y][x] = rgb;
            // Debugging print
            // cout << "x=" << x << ",y=" << y << ' ';
         }
      }
   }
   double end = omp_get_wtime();
   printf("Parallel time: %f seconds\n", end - start);

   prtRgb(channelMap.size(), maxWidth, fileDurationInTicks, &colorMap[0][0][0], "rgb.dat");
   // To see the visualization, navigate to the bin folder and type in the terminal the following:
   // gnuplot gnuplot.gnu

   return 0;
}

float scaleDown(int duration) {
   return duration / 1000.0;
}

void prtRgb(int instrCount, int maxWidth, int numTicks, RGB *colorMap1, string fname) {
   ofstream datFile(fname);
   // datFile << "x\ty\tR\tG\tB\n";

   for (int c=0; c<instrCount; c++) { //loop through channels that are used
      for (int cRow=0; cRow<maxWidth; cRow++) { //loop through the rows of each channel
         for (int t=0; t<numTicks; t++){
            datFile << dec << t << "\t" << c*maxWidth+cRow << "\t" ; // x and y
            RGB rgb = *(colorMap1 + c*maxWidth*numTicks + cRow*numTicks + t);
            datFile << rgb.R << "\t" << rgb.G << "\t" << rgb.B << "\n";
         }
      }
   }
   datFile.close();
}
