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
   MidiEvent* mev;
   int id, event, channel, channelId, xStart, xEnd, y;

   map<int, int> channelSound; //mapping from channel to sound number 
   map<int, int> channelIdx; //mapping from channel to its index in colorMap
   map<int, int> channelWidths; //mapping from channel index in colorMap to its width in colorMap

   // map<int, vector<int>> channelMap; //mapping from channel idx to [sound number, channel width]
   // map<int, int> channelIdx; // mapping from channel number to its index in colorMap
   int eventItr = 0;
   int numEvents = midifile[0].size();

   // Get the number of instruments and their channel designation
   while (eventItr < numEvents && !midifile[0][eventItr].isNoteOn()) {
      mev = &midifile[0][eventItr];
      if (mev->getCommandNibble() == 192) { //a program change event, which sets up a sound on a channel
         int channel = mev->getChannel();
         int instr = (int)(*mev)[1];
         vector<int> info = {instr, 1}; // channel width is initialized to 1
         channelSound.insert(pair<int, int>(channel, instr));
         channelWidths.insert(pair<int, int>(channel, 1)); // channel width is initialized to 1
         channelIdx.insert(pair<int, int>(channel, channelSound.size()-1));
         //Debugging print
         // cout << "Set channel " << channel << " to " << (int)(*mev)[1] << endl;
      }
      eventItr++;
   }

   if (channelSound.size() == 0) { // if there is no explicit designation of an instrument to a channel
      channelSound.insert(pair<int, int>(0, 0)); //default the only channel to 'acoustic grand piano'
      channelWidths.insert(pair<int, int>(0, 1));
      channelIdx.insert(pair<int, int>(0, 0));
   }

   int fileDurationInTicks = midifile.getFileDurationInTicks();
   cout << "Real file duration: " << fileDurationInTicks << endl;

   float scale = scaleDown(fileDurationInTicks);
   fileDurationInTicks /= scale;
   int maxWidth = 1;

   //OpenMP variables
   map<int, vector<MidiEvent>> channelEvents;
   // vector<MidiEvent> myEventList;
   RGB defaultRGB; // default color is white. See utils.h > RGB struct
   vector<vector<vector<RGB>>> colorMap(channelSound.size(), vector<vector<RGB>>(10, vector<RGB>(fileDurationInTicks, defaultRGB)));
   //set num thread
   //split the one track into channels of noteOn events, in parallel

   //This for loop is essentially grouping the note-on events by their channels
   for (event=eventItr; event<numEvents; event++) { //parallel loop chunks of 1
      mev = &midifile[0][event];
      channel = mev -> getChannel();
      channelId = channelIdx[channel];
      // #pragma omp critical  
      // {
      if (channelEvents.find(channelId)==channelEvents.end()) { // not found
         vector<MidiEvent> events = {*mev};
         channelEvents[channelId] = events;
      } else {
         channelEvents[channelId].push_back(*mev);
      }
      // }
   }

   omp_set_num_threads(channelSound.size());
   int numThreads = omp_get_num_threads();

   double start = omp_get_wtime();
   vector<MidiEvent> eventsFromChannel;
   MidiEvent eObj;

   #pragma omp parallel shared(numEvents, numThreads, channelEvents, channelSound, channelWidths, midifile, colorMap, scale) \
   private(id, eObj, event, eventsFromChannel, xStart, xEnd, y) reduction(max:maxWidth) default(none) 
   {
      id = omp_get_thread_num();
      eventsFromChannel = channelEvents[id];
      for (event = 0; event<eventsFromChannel.size(); event++) {
         eObj = eventsFromChannel[event];
         if (eObj.isNoteOn()) {
            xStart = (eObj.tick)/scale;
            xEnd = xStart + (eObj.getTickDuration())/scale;
            y = 0;
            while (y<9 && (colorMap[id][y][xStart].R!=255 || 
               colorMap[id][y][xStart].G!=255 || 
               colorMap[id][y][xStart].B!=255)) { //the starting cell is painted 
               y++;    //proceed to the next row
               if (y == channelWidths[id]) {
                  channelWidths[id]++;
                  maxWidth = max(channelWidths[id], maxWidth);
               }
            }

            float hue = (float)channelSound[id] * 2.8125; // hue ∈ [0,359], instrument ∈ [0,127]
            float val = ((float) eObj[1]) / 127.0 * 100.0; // value ∈ [0,100] , pitch ∈ [0, 127]
            float sat = ((float) eObj[2]) / 127.0 * 100.0; // saturation ∈ [0,100] , velocity (~volume) ∈ [0, 127]
            RGB rgb = HSVtoRGB(hue, sat, val);

            for (int x=xStart; x<=xEnd; x++) {
               colorMap[id][y][x] = rgb;
               // Debugging print
               // cout << "x=" << x << ",y=" << y << ' ';
            }
         } 
      }
   }
   double end = omp_get_wtime();
   cout << "Scaled file duration: " << fileDurationInTicks << endl;
   cout << "Instrument count: " << channelSound.size() << endl;
   cout << "Max width: " << maxWidth << endl;
   cout << "Events count: " << numEvents << endl;
   cout << "=> Problem size (Events count + (Instrument count * Max width * Scaled file duration)): " 
         << channelSound.size()*maxWidth*fileDurationInTicks + numEvents << endl;
   printf("Parallel time: %f seconds\n", end - start);

   // prtRgb(channelMap.size(), maxWidth, fileDurationInTicks, &colorMap[0][0][0], "rgb.dat");
   printRgb(channelWidths, channelIdx, fileDurationInTicks, colorMap, "rgb.dat");
   // To see the visualization, navigate to the bin folder and type in the terminal the following:
   // gnuplot gnuplot.gnu

   return 0;
}

float scaleDown(int duration) {
   return duration / 1000.0;
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
         cout << "current Y: " << currentY << ", channelRow: " << channelRow << "\n";
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
