import struct, sys, math
from optparse import OptionParser
from collections import namedtuple


NUM_SAMPLES = 256

phase_mix_degree = 0
tremolo_count = 0
tremolo_state = 1

prev_low_pass_results = [0, 0]
prev_low_pass_deltas = [0, 0]
prev_high_pass_results = [0, 0]
prev_high_pass_cleans = [0, 0]


def modify_data(curr_buff, prev_buff, prev_mod_buff, prevv_mod_buff, prevvv_mod_buff, prevvvv_mod_buff, options, lr_index):
    global phase_mix_degree
    global tremolo_count
    global tremolo_state
    global prev_low_pass_results
    global prev_low_pass_deltas
    global prev_high_pass_results
    global prev_high_pass_cleans
    
    # start with a copy
    res = curr_buff[:]
    
    # overdrive
    over_level = options.overdrive
    
    # low-pass and high-pass filter
    alpha_low = options.lowpass
    alpha_high = options.hipass
    
    ### todo - work on the distortion level...
    # distortion
    dist_level = int(2**(options.dist*3.0/64.0))
    
    # phaser/flanger
    phase_mix = options.phase_mix
    
    # tremolo
    tremolo = options.tremolo
    
    #prev_sample = prev_buff[-1]
    #prev_mod_sample = prev_mod_buff[-1]
    cnt = 0
    while cnt < NUM_SAMPLES:
        # overdrive
        res[cnt] = int(res[cnt] * over_level)
        
        # distortion
        res[cnt] = ((res[cnt]+dist_level/2) / dist_level) * dist_level
        
        # limit values
        if res[cnt] >= 32768:
            res[cnt] = 32767
        elif res[cnt] < -32768:
            res[cnt] = -32768
        
        # # high pass
        # res[cnt] = int(alpha_high * (prev_mod_sample + res[cnt]-prev_sample))
        
        # # low pass (first degree)
        # res[cnt] = prev_mod_sample + int(alpha_low*(res[cnt]-prev_mod_sample))
        # # low pass (third degree) - THIS ONE BLOWS UP. DON'T USE.
        # #res[cnt] = int(alpha_low * res[cnt] + (1-alpha_low) * (3*prev_mod_sample - 3*prevv_mod_sample + prevvv_mod_sample))
        
        # low-pass first, high-pass next. low-pass result is high-pass clean
        prev_high_pass_cleans[lr_index] = prev_low_pass_results[lr_index]
        
        # low-pass with resonance
        prev_low_pass_deltas[lr_index] *= options.resonance
        prev_low_pass_deltas[lr_index] += int(alpha_low*(res[cnt] - prev_low_pass_results[lr_index]))
        if prev_low_pass_deltas[lr_index] >= 65536:
            prev_low_pass_deltas[lr_index] = 65535
        elif prev_low_pass_deltas[lr_index] < -65536:
            prev_low_pass_deltas[lr_index] = -65536
        res[cnt] = prev_low_pass_results[lr_index] + prev_low_pass_deltas[lr_index]
        if res[cnt] >= 32768:
            res[cnt] = 32767
        elif res[cnt] < -32768:
            res[cnt] = -32768
        prev_low_pass_results[lr_index] = res[cnt]
        
        # high-pass
        res[cnt] = int((prev_high_pass_results[lr_index] + res[cnt]-prev_high_pass_cleans[lr_index])*alpha_high)
        
        # limit values and continue
        if res[cnt] >= 32768:
            res[cnt] = 32767
        elif res[cnt] < -32768:
            res[cnt] = -32768
        prev_high_pass_results[lr_index] = res[cnt]
        
        phase_shift = (options.phase_low + options.phase_high) / 2.0
        phase_shift += math.sin(phase_mix_degree) * (options.phase_high - options.phase_low) / 2.0
        phase_shift = int(phase_shift)
        phase_mix_degree += options.phase_oscil
        if cnt >= phase_shift:
            mixme = res[cnt-phase_shift]
        elif cnt >= phase_shift-NUM_SAMPLES:
            mixme = prev_mod_buff[cnt+NUM_SAMPLES-phase_shift]
        elif cnt >= phase_shift-NUM_SAMPLES*2:
            mixme = prevv_mod_buff[cnt+NUM_SAMPLES*2-phase_shift]
        elif cnt >= phase_shift-NUM_SAMPLES*3:
            mixme = prevvv_mod_buff[cnt+NUM_SAMPLES*3-phase_shift]
        elif cnt >= phase_shift-NUM_SAMPLES*4:
            mixme = prevvvv_mod_buff[cnt+NUM_SAMPLES*4-phase_shift]
        else:
            raise ValueError("Wrong value of phase_shift: %d" % phase_shift)
        res[cnt] = int(phase_mix*res[cnt] + (1-phase_mix)*mixme)
        
        #prev_mod_sample = res[cnt]
        #prev_sample = curr_buff[cnt]
        
        # do the tremolo after all the other shit
        if tremolo:
            if tremolo_count >= tremolo:
                tremolo_state ^= 1
                tremolo_count = 0
            tremolo_count += 1
            res[cnt] = res[cnt] * tremolo_state
        
        cnt += 1
    
    return res


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-l", "--low-pass", dest="lowpass", type="float", default=1,
                      help="alpha (between zero and one) for a low-pass filter")
    parser.add_option("-r", "--resonance", dest="resonance", type="float", default=0,
                      help="alpha (between zero and one) for a low-pass resonance")
    parser.add_option("-i", "--high-pass", dest="hipass", type="float", default=1,
                      help="alpha (between zero and one) for a high-pass filter")
    parser.add_option("-d", "--dist", dest="dist", default=0, type="int",
                      help="distortion level (0 to 255)")
    parser.add_option("--over", dest="overdrive", default=1, type="float",
                      help="overdrive level (1 to 5)")
    parser.add_option("--phase-low", dest="phase_low", default=0, type="int",
                      help="low phase shift (0 to 1000)")
    parser.add_option("--phase-high", dest="phase_high", default=0, type="int",
                      help="high phase shift (0 to 1000)")
    parser.add_option("--phase-mix", dest="phase_mix", default=1, type="float",
                      help="phaser mix level (0 to 1)")
    parser.add_option("--phase-oscil", dest="phase_oscil", default=0.25, type="float",
                      help="phaser oscillator increments")
    parser.add_option("-t", "--tremolo", dest="tremolo", default=0, type="int",
                      help="tremolo intervals")
    parser.add_option("-o", "--out", dest="outfile",
                      help="output filename")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        sys.exit()
    
    # validate the values
    if options.lowpass > 1 or options.lowpass < 0:
        print "low pass value should be between zero and one"
        sys.exit()
    if options.resonance > 1 or options.resonance < 0:
        print "resonance value should be between zero and one"
        sys.exit()
    if options.hipass > 1 or options.hipass < 0:
        print "high pass value should be between zero and one"
        sys.exit()
    if options.dist < 0 or options.dist > 255:
        print "distortion value should be between 0 and 255"
        sys.exit()
    if options.overdrive < 1 or options.overdrive > 3:
        print "overdrive value should be between one and three"
        sys.exit()
    if options.phase_low < 0 or options.phase_low > 1000:
        print "low phase shift value should be between 0 and 1000"
        sys.exit()
    if options.phase_high < 0 or options.phase_high > 1000:
        print "high phase shift value should be between 0 and 1000"
        sys.exit()
    if options.phase_low > options.phase_high:
        print "low phase shift value cannot be higher than high phase shift value"
        sys.exit()
    if options.phase_mix > 1 or options.phase_mix < 0:
        print "phase mix value should be between zero and one"
        sys.exit()
    
    infile = open(args[0], "rb")
    if options.outfile:
        outfile = open(options.outfile, "wb")
    else:
        outfile = open(args[0]+".out", "wb")
    
    hdr = infile.read(0x2c)
    if len(hdr) < 0x2c:
        print "file is too short"
        infile.close()
        outfile.close()
        sys.exit()
    
    outfile.write(hdr)
    
    WaveHdr = namedtuple('WaveHdr', 'tag total_len fmt_tag bits a b rate c d e data_tag data_len')
    unpacked_hdr = WaveHdr._make(struct.unpack("<4sI8sIHHIIHH4sI", hdr))
    
    prev_buff_left = [0]*NUM_SAMPLES
    prev_buff_right = [0]*NUM_SAMPLES
    prev_mod_buff_left = [0]*NUM_SAMPLES
    prev_mod_buff_right = [0]*NUM_SAMPLES
    prevv_mod_buff_left = [0]*NUM_SAMPLES
    prevv_mod_buff_right = [0]*NUM_SAMPLES
    prevvv_mod_buff_left = [0]*NUM_SAMPLES
    prevvv_mod_buff_right = [0]*NUM_SAMPLES
    prevvvv_mod_buff_left = [0]*NUM_SAMPLES
    prevvvv_mod_buff_right = [0]*NUM_SAMPLES
    
    data_len = unpacked_hdr.data_len
    
    while data_len >= NUM_SAMPLES*2*2:
        curr_buff = infile.read(NUM_SAMPLES*2*2)
        data_len -= len(curr_buff)
        
        # turn into numbers, separating left and right
        curr_buff_left = list(struct.unpack("<" + "hxx"*NUM_SAMPLES, curr_buff))
        curr_buff_right = list(struct.unpack("<" + "xxh"*NUM_SAMPLES, curr_buff))
        mod_buff_left = modify_data(
            curr_buff_left, prev_buff_left,
            prev_mod_buff_left, prevv_mod_buff_left, prevvv_mod_buff_left, prevvvv_mod_buff_left,
            options, 0
        )
        mod_buff_right = modify_data(
            curr_buff_right, prev_buff_right,
            prev_mod_buff_right, prevv_mod_buff_right, prevvv_mod_buff_right, prevvvv_mod_buff_right,
            options, 1
        )
        
        # current -> previous
        prev_buff_left = curr_buff_left
        prev_buff_right = curr_buff_right
        prevvvv_mod_buff_left = prevvv_mod_buff_left
        prevvvv_mod_buff_right = prevvv_mod_buff_right
        prevvv_mod_buff_left = prevv_mod_buff_left
        prevvv_mod_buff_right = prevv_mod_buff_right
        prevv_mod_buff_left = prev_mod_buff_left
        prevv_mod_buff_right = prev_mod_buff_right
        prev_mod_buff_left = mod_buff_left
        prev_mod_buff_right = mod_buff_right
        
        # interleave the modified buffers and write them to the output file
        mod_buff = zip(mod_buff_left, mod_buff_right)
        mod_buff = map(list, mod_buff)
        mod_buff = reduce(lambda x,y: x+y, mod_buff, [])
        mod_buff = struct.pack("<" + "hh"*NUM_SAMPLES, *mod_buff)
        outfile.write(mod_buff)
    
    # handle the remainder
    outfile.write(infile.read())
    outfile.close()
    infile.close()
