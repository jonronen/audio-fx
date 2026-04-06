#!/usr/bin/python
import struct, sys, math, numpy, functools
from optparse import OptionParser
from collections import namedtuple


# TODO: refactor these definitions to be class variables
NUM_SAMPLES = 1024
BITS = 16
MAX_SAMPLE = 2**(BITS-1)

HANN_WINDOW = numpy.array([math.sin((math.pi*n)/NUM_SAMPLES)**2 for n in range(NUM_SAMPLES)])
HANN_WINDOW_DOUBLE = numpy.array([math.sin((math.pi*n)/NUM_SAMPLES/2)**2 for n in range(NUM_SAMPLES*2)])

class Context:
    def __init__(self):
        self.prev_buff = numpy.array([0] * NUM_SAMPLES, dtype = complex)
        self.prev_mod_buffs = [numpy.array([0] * NUM_SAMPLES * 2, dtype = complex) for i in range(4)]

def extrapolate(buff):
    return numpy.array([buff[i//2] for i in range(2*buff.size)])

def get_windowed_quarters(buff, start_quarter):
    l = numpy.array(([0] * (NUM_SAMPLES//2)) + list(buff * HANN_WINDOW_DOUBLE) + ([0] * (NUM_SAMPLES//2)))
    return l[(start_quarter + 1) * NUM_SAMPLES//2 : (start_quarter + 3) * NUM_SAMPLES//2]

# TODO: refactor this to be a class method
def modify_data(curr_buff, context, freqs):
    # start with a copy of the new buffer, in the form of numpy.ndarray
    new_buff = numpy.array(list(curr_buff), dtype = complex)
    
    # convert current sample to a complex number with real part in the range [-1,1]
    new_buff = new_buff / MAX_SAMPLE
    
    # create the mid-buffer, composed of the last half of the previous buffer and the first half of the new buffer
    mid_buff = numpy.array(context.prev_buff[NUM_SAMPLES//2:].tolist() + new_buff[:NUM_SAMPLES//2].tolist())
    
    # stretch the arrays
    new_mod_buff = extrapolate(new_buff)
    mid_mod_buff = extrapolate(mid_buff)
    
    res = get_windowed_quarters(context.prev_mod_buffs[0], 3) + \
        get_windowed_quarters(context.prev_mod_buffs[1], 2) + \
        get_windowed_quarters(context.prev_mod_buffs[2], 1) + \
        get_windowed_quarters(context.prev_mod_buffs[3], 0) + \
        get_windowed_quarters(mid_mod_buff, -1)
    
    context.prev_buff = new_buff
    context.prev_mod_buffs[0] = context.prev_mod_buffs[2]
    context.prev_mod_buffs[1] = context.prev_mod_buffs[3]
    context.prev_mod_buffs[2] = mid_mod_buff
    context.prev_mod_buffs[3] = new_mod_buff
    return list(map(lambda x: int(x.real), res * MAX_SAMPLE))


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-o", "--out", dest="outfile",
                      help="output filename")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        sys.exit()
    
    infile = open(args[0], "rb")
    if options.outfile:
        outfile = open(options.outfile, "wb")
    else:
        outfile = open(args[0]+".out", "wb")
    
    hdr = infile.read(0x2c)
    if len(hdr) < 0x2c:
        print("file is too short")
        infile.close()
        outfile.close()
        sys.exit()
    
    outfile.write(hdr)
    
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
    
    contexts = [Context(), Context()]
    
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
        mod_buff_left = modify_data(curr_buff_left, contexts[0], freqs)
        if unpacked_hdr.ch > 1:
            mod_buff_right = modify_data(curr_buff_right, contexts[1], freqs)
        else:
            mod_buff_right = [0] * NUM_SAMPLES
        
        if unpacked_hdr.ch > 1:
            # interleave the modified buffers and write them to the output file
            mod_buff = zip(mod_buff_left, mod_buff_right)
            mod_buff = map(list, mod_buff)
            mod_buff = functools.reduce(lambda x,y: x+y, mod_buff, [])
        else:
            mod_buff = mod_buff_left
        
        # prevent clipping and align the samples in case there's an overflow/underflow
        mod_buff = list(map(lambda x: max(min(x, 32767), -32768), mod_buff))

        if unpacked_hdr.ch == 1:
            mod_buff = struct.pack("<" + "h"*NUM_SAMPLES, *mod_buff)
        else:
            mod_buff = struct.pack("<" + "hh"*NUM_SAMPLES, *mod_buff)
        outfile.write(mod_buff)
    
    # handle the remainder
    outfile.write(infile.read())
    outfile.close()
    infile.close()
