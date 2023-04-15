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
void printRgb(map<int, int> channelWidths, map<int, int> channelIdx, int numTicks, vector<vector<vector<RGB>>>& colorMap, string fname);

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   MidiFile midifile;
   if (options.getArgCount() == 0) midifile.read(cin);
   else midifile.read(options.getArg(1));
   midifile.joinTracks(); // midifile.getTrackCount() will now return 1
   midifile.linkNotePairs();

   // MIDI variables
   MidiEvent* mev1;
   map<int, int> channelSound; //mapping from channel to sound number 
   map<int, int> channelIdx; //mapping from channel to its index in colorMap
   map<int, int> channelWidths; //mapping from channel index in colorMap to its width in colorMap
   int eventItr = 0;
   int numEvents = midifile[0].size();

   // Get the number of instruments and their channel designation
   while (eventItr < numEvents && !midifile[0][eventItr].isNoteOn()) {
      mev1 = &midifile[0][eventItr];
      if (mev1->getCommandNibble() == 192) { //a program change event, which sets up a sound on a channel
         int channel = mev1->getChannel();
         int instr = (int)(*mev1)[1];
         channelSound.insert(pair<int, int>(channel, instr));
         channelWidths.insert(pair<int, int>(channel, 1)); // channel width is initialized to 1
         channelIdx.insert(pair<int, int>(channel, channelSound.size()-1));
         //Debugging print
         // cout << "Set channel " << channel << " to " << (int)(*mev1)[1] << endl;
      }
      eventItr++;
   }

   if (channelSound.size() == 0) { // if there is no explicit designation of an instrument to a channel
      channelSound.insert(pair<int, int>(0, 0)); //default the only channel to 'acoustic grand piano'
      channelWidths.insert(pair<int, int>(0, 1));
      channelIdx.insert(pair<int, int>(0, 0));
   }

   int fileDurationInTicks = midifile.getFileDurationInTicks();
   RGB defaultRGB; // default color is white. See utils.h > RGB struct
   vector<vector<vector<RGB>>> colorMap(channelSound.size(), vector<vector<RGB>>(10, vector<RGB>(fileDurationInTicks, defaultRGB)));
   int maxWidth = 1;
   cout << "Real file duration: " << fileDurationInTicks << endl;

   float scale = scaleDown(fileDurationInTicks);
   fileDurationInTicks /= scale;

   //OpenMP variables
   int numThreads = 16;
   MidiEvent *mev;
   int id, event, channel, channelId, xStart, xEnd, y;

   //Start timing
   double start = omp_get_wtime();

   #pragma omp parallel shared(numEvents, numThreads, eventItr, channelIdx, channelSound, channelWidths, midifile, colorMap, scale) \
   private(id, event, mev, channel, channelId, xStart, xEnd, y) reduction(max:maxWidth) default(none) num_threads(numThreads)
   {
      id = omp_get_thread_num();
      for (event = eventItr + id; event<numEvents; event+=numThreads) {
         mev = &midifile[0][event];
         if (mev->isNoteOn()) {
            xStart = (mev->tick)/scale;
            xEnd = xStart + (mev->getTickDuration())/scale;

            channel = mev->getChannel();
            y = 0;
            channelId = channelIdx[channel]; 
            while (y<9 && (colorMap[channelId][y][xStart].R!=255 || 
                  colorMap[channelId][y][xStart].G!=255 || 
                  colorMap[channelId][y][xStart].B!=255)) { //the starting cell is painted 
               y++;
               // #pragma omp critical
               if (y == channelWidths[channel]) {
                  channelWidths[channel]++;
                  maxWidth = max(channelWidths[channel], maxWidth);
               }
            }
            float hue = (float) channelSound[channel] * 2.8125; // hue ∈ [0,359], instrument ∈ [0,127]
            float val = ((float) (*mev)[1]) / 127.0 * 100.0; // value ∈ [0,100] , pitch ∈ [0, 127]
            float sat = ((float) (*mev)[2]) / 127.0 * 100.0; // saturation ∈ [0,100] , velocity (~volume) ∈ [0, 127]
            RGB rgb = HSVtoRGB(hue, sat, val);

            for (int x=xStart; x<=xEnd; x++) {
               // #pragma omp critical
               colorMap[channelId][y][x] = rgb;
               //Debugging print
               // cout << "x=" << x << ",y=" << y << ' ';
            }
         }
      }
   }

   int totalNoteOns = 0;
   int totalCellsPainted = 0;
   for (int event = eventItr; event<numEvents; event++) {
      MidiEvent *mev = &midifile[0][event];
      if (mev->isNoteOn()) {
         int xStart = (mev->tick)/scale;
         int xEnd = xStart + (mev->getTickDuration())/scale;
         totalCellsPainted += xEnd - xStart;
         totalNoteOns++;
      }
   }

   double end = omp_get_wtime();
   int totalWidths = 0;
   for (int i=0; i<channelWidths.size(); i++) {
      totalWidths += channelWidths[i];
   }

   int instrCount = channelSound.size();
   cout << "Scaled file duration: " << fileDurationInTicks << endl;
   cout << "Instrument count: " << instrCount << endl;
   cout << "Max width: " << maxWidth << endl;
   cout << "Total width: " << totalWidths << endl;
   cout << "Events count: " << numEvents << endl;
   cout << "=> Events count + (Total width * Scaled file duration)): " 
         << totalWidths*fileDurationInTicks + numEvents << endl;
   cout << "Problem size = Total cells painted: " << totalCellsPainted << endl;
   cout << "Problem size = Total NOTE-ON events: " << totalNoteOns << endl;
   printf("Parallel time: %f seconds, run with %d thread(s).\n", end - start, numThreads);

   printRgb(channelWidths, channelIdx, fileDurationInTicks, colorMap, "rgb.dat");
   // prtRgb(instrCount, maxWidth, fileDurationInTicks, &colorMap[0][0][0], "rgb.dat");
   // To see the visualization, navigate to the bin folder and type in the terminal the following:
   // gnuplot gnuplot.gnu

   return 0;
}

float scaleDown(int duration) {
   return duration / 500.0;
}

void printRgb(map<int, int> channelWidths, map<int, int> channelIdx, int numTicks, vector<vector<vector<RGB>>>& colorMap, string fname) {
   ofstream datFile(fname);
   // int instrCount = channelIdx.size();   
   int channelY = 0;
   for (int chIdx=0; chIdx<channelIdx.size(); chIdx++) {
      int channelNum;

      for (auto const& entry: channelIdx) {
         if (entry.second == chIdx) {
            channelNum = entry.first;
         }
      }

      int channelWidth = channelWidths[channelNum];

      for (int channelRow=0; channelRow<channelWidth; channelRow++) { //loop through the rows of each channel
         int currentY = channelY + channelRow;
         // cout << "current Y: " << currentY << ", channelRow: " << channelRow << "\n";
         for (int t=0; t<numTicks; t++){
            RGB rgb = colorMap[chIdx][channelRow][t];
            // cout << "(" << rgb.R << ", " << rgb.G << ", " << rgb.B << ") \t";
            datFile << dec << t << "\t" << currentY << "\t" ; // x and y
            datFile << rgb.R << "\t" << rgb.G << "\t" << rgb.B << "\n";
         }
      }
      channelY += (chIdx + 1) * channelWidth;
   }
   // fclose(fp);
   datFile.close();
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
