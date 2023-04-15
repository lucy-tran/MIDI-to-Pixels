struct RGB {
    unsigned int R=255;       // ∈ [0, 255]
    unsigned int G=255;       // ∈ [0, 255]
    unsigned int B=255;       // ∈ [0, 255]
};

RGB HSVtoRGB(float H, float S, float V);

void getFileAndNumThreads(int argc, char *argv[], std::string * fileName, int * nThreads);
void getArguments(int argc, char *argv[], int * N, int * nThreads);
