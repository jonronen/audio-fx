#!/usr/bin/python
import struct, sys, math
from optparse import OptionParser
from collections import namedtuple

SAMPLE_RATE = 44100
TOTAL_SAMPLES = SAMPLE_RATE * 5 # 5 seconds
BITS = 16
CHANNELS = 1 # mono
HEADER_SIZE = 44 # fixed WAV header


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-f", "--freq", dest="freq", default=300, type="int",
                      help="wave frequency")
    parser.add_option("-o", "--out", dest="outfile",
                      help="output filename")

    (options, args) = parser.parse_args()
    
    if options.freq > 10000 or options.freq < 40:
        print("Invalid frequency %d, please choose a value between 40 and 10000" % options.freq)
        sys.exit()
    
    if options.outfile:
        outfile = open(options.outfile, "wb")
    else:
        outfile = open("out.wav", "wb")
    
    packed_hdr = struct.pack("<4sI8sIHHIIHH4sI",
        b"RIFF",
        HEADER_SIZE + TOTAL_SAMPLES * BITS // 8,
        b"WAVEfmt ",
        BITS,
        1, # 1 = PCM
        CHANNELS,
        SAMPLE_RATE,
        SAMPLE_RATE * BITS * CHANNELS // 8,
        BITS * CHANNELS // 8,
        BITS,
        b"data",
        TOTAL_SAMPLES * BITS // 8
        )

    outfile.write(packed_hdr)
    
    for sample_idx in range(TOTAL_SAMPLES):
        sample_val = math.sin(sample_idx / SAMPLE_RATE * options.freq * 2 * math.pi)
        sample_val *= (2 ** (BITS - 4))
        outfile.write(struct.pack("<h", int(sample_val)) * CHANNELS)
    
    outfile.close()

