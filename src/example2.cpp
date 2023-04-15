#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace smf;

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   MidiFile midifile;
   if (options.getArgCount() == 0) midifile.read(cin);
   else midifile.read(options.getArg(1));
   midifile.doTimeAnalysis();
   midifile.linkNotePairs();

   int tracks = midifile.getTrackCount();
   cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
   if (tracks > 1) cout << "TRACKS: " << tracks << endl;
   int totalNoteOnCount = 0;
   int totalEventCount = 0;
   for (int track=0; track<tracks; track++) {
      if (tracks > 1) cout << "\nTrack " << track << endl;
      int noteOnCount = 0;
      // cout << "Tick\tSeconds\tDur\tMessage" << endl;
      for (int event=0; event<midifile[track].size(); event++) {
      //    cout << dec << midifile[track][event].tick;
      //    cout << '\t' << dec << midifile[track][event].seconds;
      //    cout << '\t';
         totalEventCount++;
         if (midifile[track][event].isNoteOn()) {
      //       cout << midifile[track][event].getDurationInSeconds()<< ' ';
      //       printf("%hhu", midifile[track][event].getChannel());
            noteOnCount++;
         }
      //    cout << '\t' << hex;
      //    for (int i=0; i<midifile[track][event].size(); i++)
      //       cout << (int)midifile[track][event][i] << ' ';
      //    cout << endl;
      }
      cout << "Note on event count: " << noteOnCount << endl;
      totalNoteOnCount += noteOnCount;
   }
   cout << "Total note on event count: " << totalNoteOnCount << endl;
   cout << "Total event count: " << totalEventCount << endl;

   return 0;
}