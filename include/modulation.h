#include <stdint.h>
#include <complex.h>
#include <vector>

// For PlutoSDR
int bpsk(std::vector<bool> &in, std::vector<int16_t> *out, int size);