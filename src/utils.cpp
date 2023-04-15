//
// Adapted from Amit Raja Kalidindi's code
// Source: https://www.codespeedy.com/hsv-to-rgb-in-cpp/
//
// Description:   Converts HSV to RGB color value. 
//                  where H ∈ [0, 359], S ∈ [0, 100], V ∈ [0, 100]
//                  and R ∈ [0, 255], G ∈ [0, 255], B ∈ [0, 255].
//

#include <bits/stdc++.h>
#include "utils.h"
#include <stdio.h>      /* fprintf, fgets */
#include <stdlib.h>     /* atoi */
#include <unistd.h> // getopt

using namespace std;

RGB HSVtoRGB(float H, float S, float V) {
    RGB rgb;
    if(H>360 || H<0 || S>100 || S<0 || V>100 || V<0){
        cout << "The given HSV values are not in valid range" << endl;
        return rgb;
    }
    float s = S/100.0;
    float v = V/100.0;
    float C = s*v;
    float X = C*(1-fabs(fmod(H/60.0, 2)-1));
    float m = v-C;
    float r,g,b;
    if(H >= 0 && H < 60){
        r = C,g = X,b = 0;
    }
    else if(H >= 60 && H < 120){
        r = X,g = C,b = 0;
    }
    else if(H >= 120 && H < 180){
        r = 0,g = C,b = X;
    }
    else if(H >= 180 && H < 240){
        r = 0,g = X,b = C;
    }
    else if(H >= 240 && H < 300){
        r = X,g = 0,b = C;
    }
    else{
        r = C,g = 0,b = X;
    }

    rgb.R = (r+m)*255;
    rgb.G = (g+m)*255;
    rgb.B = (b+m)*255;

    //Debugging print
    // printf("(");
    // printf("%hhu, ", rgb.R);
    // printf("%hhu, ", rgb.G);
    // printf("%hhu", rgb.B);
    // printf(") ");

    return rgb;
}

// process command line looking for the number of characters
// in the input string as our 'problem size'. Set the value of N
// of N to that number or generate error if not provided.
//   see:
// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
//
void getFileAndNumThreads(int argc, char *argv[], string * fileName, int * nThreads) {
    char *tvalue;
    int c;        // result from getopt calls
    int tflag = 0;
    int fflag = 0;
    cout << "Here" << endl;
    while ((c = getopt (argc, argv, "f:t:")) != -1) {
      switch (c)
        {
        case 'f':
            cout << "get -f ";
            fflag = 1;
            *fileName = argv[1];
            break;    
        case 't':
            tflag = 1;
            tvalue = optarg;
            *nThreads = atoi(tvalue);
            break;
        case '?':
            if (optopt == 'n') {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            } else if (isprint (optopt)) {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            } else {
                fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
                exit(EXIT_FAILURE);
            }
        }
    }
}

// process command line looking for the number of characters
// in the input string as our 'problem size'. Set the value of N
// of N to that number or generate error if not provided.
//   see:
// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
//
void getArguments(int argc, char *argv[], int * N, int * nThreads) {
    char *nvalue;
    char *tvalue;
    int c;        // result from getopt calls
    int nflag = 0;
    int tflag = 0;

    while ((c = getopt (argc, argv, "t:n:")) != -1) {
      switch (c)
        {
        case 'n':
          nflag = 1;
          nvalue = optarg;
          *N = atoi(nvalue);
          break;

        case 't':
          tflag = 1;
          tvalue = optarg;
          *nThreads = atoi(tvalue);
          break;

        case '?':
          if (optopt == 'n') {
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          } else if (isprint (optopt)) {
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          } else {
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
            exit(EXIT_FAILURE);
          }

        }
    }
    if (nflag == 0) {
      fprintf(stderr, "Usage: %s -n size\n", argv[0]);
      exit(EXIT_FAILURE);
    }
}
