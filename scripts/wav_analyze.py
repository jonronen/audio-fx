#!/usr/bin/python
import struct, sys, math, numpy
from scipy.fft import fft, fftfreq
from optparse import OptionParser
from collections import namedtuple


NUM_SAMPLES = 1024
BITS = 16
MAX_SAMPLE = 2**(BITS-1)

HANN_WINDOW = numpy.array([math.sin((math.pi*n)/NUM_SAMPLES)**2 for n in range(NUM_SAMPLES)])

def analyze_data_chunk(curr_buff, freqs):
    # start with a copy of the new buffer, in the form of numpy.ndarray
    new_buff = numpy.array(list(curr_buff), dtype = complex)
    
    # convert current sample to a complex number with real part in the range [-1,1]
    new_buff = new_buff / MAX_SAMPLE
    
    freqs += numpy.abs(fft(new_buff * HANN_WINDOW)[:NUM_SAMPLES//2])


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog WAVFILE")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        sys.exit()
    
    infile = open(args[0], "rb")
    
    hdr = infile.read(0x2c)
    if len(hdr) < 0x2c:
        print("file is too short")
        infile.close()
        outfile.close()
        sys.exit()
    
    WaveHdr = namedtuple(
        'WaveHdr',
        'tag total_len fmt_tag bits a ch rate bt algn bps data_tag data_len'
    )
    unpacked_hdr = WaveHdr._make(struct.unpack("<4sI8sIHHIIHH4sI", hdr))
    print("Unpacked header:", unpacked_hdr)
    if unpacked_hdr.tag != b"RIFF" or \
            unpacked_hdr.fmt_tag != b"WAVEfmt " or \
            unpacked_hdr.data_tag != b"data":
        print("input file is not a wav file")
        sys.exit()
    if unpacked_hdr.bits != BITS or unpacked_hdr.bps != BITS:
        print("expected 16-bit wav file")
        sys.exit()
    
    freqs = numpy.array([0] * (NUM_SAMPLES//2), dtype = float)
    
    data_len = unpacked_hdr.data_len
    
    while data_len >= NUM_SAMPLES*2*unpacked_hdr.ch:
        curr_buff = infile.read(NUM_SAMPLES*2*unpacked_hdr.ch)
        data_len -= len(curr_buff)
        
        # turn into numbers, separating left and right if needed
        if unpacked_hdr.ch == 1:
            curr_buff_left = list(struct.unpack(
                "<" + "h"*NUM_SAMPLES, curr_buff
            ))
            curr_buff_right = [0] * NUM_SAMPLES
        else:
            curr_buff_left = list(struct.unpack(
                "<" + "hxx"*NUM_SAMPLES, curr_buff
            ))
            curr_buff_right = list(struct.unpack(
                "<" + "xxh"*NUM_SAMPLES, curr_buff
            ))
        analyze_data_chunk(curr_buff_left, freqs)
        if unpacked_hdr.ch > 1:
            analyze_data_chunk(curr_buff_right, freqs)
    
    # handle the remainder
    infile.close()
    
    T = 1.0 / unpacked_hdr.rate # Sampling interval
    t = numpy.linspace(0.0, NUM_SAMPLES*T, NUM_SAMPLES, endpoint=False)
    xf = fftfreq(NUM_SAMPLES, T)[:NUM_SAMPLES//2] # Get positive frequencies only
    
    import matplotlib.pyplot as plt
    plt.plot(xf, 2.0/NUM_SAMPLES * freqs)
    plt.xscale('log')
    plt.grid()
    plt.show()
