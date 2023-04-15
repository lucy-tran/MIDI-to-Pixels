#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <fstream>

// From the midifile library
#include "MidiFile.h" 
#include "Options.h"

#include "utils.h"
#include "seq_time.h"  // Libby's timing function that is similar to omp style

using namespace smf;
using namespace std;

float scaleDown(int fileDuration);
void prtRgb(int instrCount, int maxWidth, int numTicks, RGB *colorMap1, string fname);
void printRgb(map<int, vector<int>> channelMap, map<int, int> channelIdx, int numTicks, vector<vector<vector<RGB>>>& colorMap, string fname);

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
   map<int, vector<int>> channelMap; //mapping from channel to sound number and its width in colorMap
   map<int, int> channelIdx; // mapping from channel to its index in colorMap
   int event = 0;
   int numEvents = midifile[0].size();

   // Get the number of instruments and their channel designation
   while (event < numEvents && !midifile[0][event].isNoteOn()) {
      mev = &midifile[0][event];
      if (mev->getCommandNibble() == 192) { //a program change event, which sets up a sound on a channel
         int channel = mev->getChannel();
         int instr = (int)(*mev)[1];
         vector<int> info = {instr, 1}; // channel width is initialized to 1
         channelMap.insert(pair<int, vector<int>>(channel, info));
         channelIdx.insert(pair<int, int>(channel, channelMap.size()-1));
         //Debugging print
         // cout << "Set channel " << channel << " to " << (int)(*mev)[1] << endl;
      }
      event++;
   }

   if (channelMap.size() == 0) { // if there is no explicit designation of an instrument to a channel
      vector<int> info = {0, 1};
      channelMap.insert(pair<int, vector<int>>(0, info)); //default the only channel to 'acoustic grand piano' (number 0)
      channelIdx.insert(pair<int, int>(0, 0));
   }

   int fileDurationInTicks = midifile.getFileDurationInTicks();
   cout << "Real file duration: " << fileDurationInTicks << endl;

   float scale = scaleDown(fileDurationInTicks);
   fileDurationInTicks /= scale;
   RGB defaultRGB; // default color is white. See utils.h > RGB struct
   vector<vector<vector<RGB>>> colorMap(channelMap.size(), vector<vector<RGB>>(10, vector<RGB>(fileDurationInTicks, defaultRGB)));
   // RGB colorMap[channelMap.size()][10][fileDurationInTicks];
   int maxWidth = 1;

   cout << "Instrument count: " << channelMap.size() << endl;
   cout << "File duration: " << fileDurationInTicks << endl;

   double start = c_get_wtime();

   for (event; event<midifile[0].size(); event++) {
      mev = &midifile[0][event];
      if (mev->isNoteOn()) {
         int xStart = (mev->tick)/scale;
         int xEnd = xStart + (mev->getTickDuration())/scale;

         int channel = mev->getChannel();
         int y = 0;
         channel = channelIdx[channel]; 
         while (y<9 && (colorMap[channel][y][xStart].R!=255 || 
               colorMap[channel][y][xStart].G!=255 || 
               colorMap[channel][y][xStart].B!=255)) { //the starting cell is painted 
            y++;
            if (y == channelMap[channel][1]) {
               channelMap[channel][1]++;
               maxWidth = max(channelMap[channel][1], maxWidth);
            }
         }

         // For now, since our demo MIDI files only have 1 instrument (1 channel) with 
         // similar volume throughout, we will maximize the differences in pitch. 
         // Conversion formula for songs with only 1 instruments, without much fluctuation
         // in volume -> we will maximize the differences in pitch
         // float hue = ((float) (*mev)[1]) / 80.0 * 360.0; // hue ∈ [0,359], pitch ∈ [0,127]
         // float val = ((float) (*mev)[1]) / 80.0 * 100.0; // value ∈ [0,100] , pitch ∈ [0, 127]
         // float sat = ((float) (*mev)[2]) / 80.0 * 100.0; // saturation ∈ [0,100] , velocity (~volume) ∈ [0, 127]

         // Conversion formula for songs with more than 1 instruments
         float hue = ((float)channelMap[channel][0]) * 2.8125; // hue ∈ [0,359], channel ∈ [0,127]
         float val = (((float) (*mev)[1]) / 127.0 * 100.0); // value ∈ [0,100], pitch ∈ [0, 127]
         float sat = ((float) (*mev)[2]) / 127.0 * 100.0; // saturation ∈ [0,100] , velocity (~volume) ∈ [0, 127]

         RGB rgb = HSVtoRGB(hue, sat, val);

         for (int x=xStart; x<=xEnd; x++) {
            colorMap[channel][y][x] = rgb;
            //Debugging print
            // cout << "x=" << x << ",y=" << y << ' ';
         }
         // cout << endl;
      }
   }
   double end = c_get_wtime();
   cout << "Scaled file duration: " << fileDurationInTicks << endl;
   cout << "Instrument count: " << channelMap.size() << endl;
   cout << "Max width: " << maxWidth << endl;
   cout << "Events count: " << numEvents << endl;
   cout << "=> Problem size (Events count + (Instrument count * Max width * Scaled file duration)): " 
         << channelMap.size()*maxWidth*fileDurationInTicks + numEvents << endl;
   printf("Sequential time: %f seconds\n", end - start);

   // prtRgb(channelMap.size(), maxWidth, fileDurationInTicks, &colorMap[0][0][0], "rgb.dat");
   printRgb(channelMap, channelIdx, fileDurationInTicks, colorMap, "rgb.dat");
   // To see the visualization, navigate to the bin folder and type in the terminal the following:
   // gnuplot gnuplot.gnu

   return 0;
}

float scaleDown(int duration) {
   return duration / 500.0;
}

void printRgb(map<int, vector<int>> channelMap, map<int, int> channelIdx, int numTicks, vector<vector<vector<RGB>>>& colorMap, string fname) {
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

      vector<int> value = channelMap[channelNum];
      int channelWidth = value[1];

      for (int channelRow=0; channelRow<channelWidth; channelRow++) { //loop through the rows of each channel
         int currentY = channelY + channelRow;
         cout << "current Y: " << currentY << ", channelRow: " << channelRow << "\n";
         for (int t=0; t<numTicks; t++){
            RGB rgb = colorMap[channelNum][channelRow][t];
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
   // fclose(fp);
   datFile.close();
}



   // cout << hex << (int)(*mev)[0] << ' ';
   // cout << dec<< (int)(*mev)[0] << ' ';
   // cout << "conNum: " << (*mev).getControllerNumber() << ' ';
   // cout << "conVal: " << (*mev).getControllerValue() << ' ';

   // cout << "commNib: " << (*mev).getCommandNibble() << ' ';
   // cout << "commByte: " << (*mev).getCommandByte() << ' ';

   // cout << "channelNib: " << (*mev).getChannelNibble() << ' ';
   // cout << "channel: " << (*mev).getChannel() << ' ';
   // cout << endl;
