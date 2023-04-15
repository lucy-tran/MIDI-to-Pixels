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
   if (options.getArgCount() > 0) midifile.read(options.getArg(1));
   else midifile.read(cin);
   cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
   cout << "TRACKS: " << midifile.getTrackCount() << endl;
   midifile.joinTracks();
   midifile.doTimeAnalysis();
   midifile.linkNotePairs();
   // midifile.getTrackCount() will now return "1", but original
   // track assignments can be seen in .track field of MidiEvent.
   cout << "TICK    DELTA   TRACK   DURATION   MIDI MESSAGE\n";
   cout << "____________________________________\n";
   MidiEvent* mev;
   int deltatick;
   for (int event=0; event < midifile[0].size(); event++) {
      mev = &midifile[0][event];
      if (event == 0) deltatick = mev->tick;
      else deltatick = mev->tick - midifile[0][event-1].tick;
      cout << dec << mev->tick;
      cout << '\t' << deltatick;
      cout << '\t' << mev->track;
      cout << '\t';
      if (mev->isNoteOn())
         cout << mev->getTickDuration();
      cout << '\t' << hex;
      for (unsigned int i=0; i < mev->size(); i++)
         cout << (int)(*mev)[i] << ' ';
      cout << endl;
   }
   return 0;
}