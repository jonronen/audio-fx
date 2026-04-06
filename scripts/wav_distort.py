#!/usr/bin/python
import struct, sys, math
from optparse import OptionParser
from collections import namedtuple


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-p", "--distortion-power", dest="dist_pow", default=1, type="float",
                      help="distortion exponent (between zero and one)")
    parser.add_option("-f", "--distortion-factor", dest="dist_fact", default=1, type="float",
                      help="distortion factor for the low notes (between zero and one)")
    parser.add_option("-o", "--out", dest="outfile",
                      help="output filename")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        sys.exit()
    
    # validate the values
    if options.dist_pow > 1 or options.dist_pow <= 0:
        print("distortion exponent should be between zero and one (received %f)" % options.dist_pow)
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
    if unpacked_hdr.tag != b"RIFF" or \
            unpacked_hdr.fmt_tag != b"WAVEfmt " or \
            unpacked_hdr.data_tag != b"data":
        print("input file is not a wav file")
        sys.exit()
    if unpacked_hdr.bits != 16 or unpacked_hdr.bps != 16:
        print("expected 16-bit wav file")
        sys.exit()
    
    data_len = unpacked_hdr.data_len
    
    while data_len > 0:
        sample_bytes = infile.read(2)
        data_len -= len(sample_bytes)
        
        # turn into number
        curr_sample = struct.unpack("<h", sample_bytes)[0]
        
        sample_sign = math.copysign(1, curr_sample)
        mod_sample = abs(curr_sample / (2 ** 15))
        mod_sample = math.pow(mod_sample, options.dist_pow)
        mod_sample *= sample_sign * (2 ** 15)
        if mod_sample < 0:
            mod_sample *= options.dist_fact
        mod_sample = int(mod_sample)

        outfile.write(struct.pack("<h", mod_sample))
    
    # handle the remainder
    outfile.write(infile.read())
    outfile.close()
    infile.close()

