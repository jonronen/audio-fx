import struct, sys
from optparse import OptionParser
from collections import namedtuple


def compute_average(li):
    return reduce(lambda x,y: x+y, li, 0)/float(len(li))

def compute_energy(li, avg):
    return compute_average(map(lambda x: (x-avg)*(x-avg)/2.0, li))


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-l", "--low-pass", dest="lowpass", type="float", default=1,
                      help="alpha (between zero and one) for a low-pass filter")
    parser.add_option("-i", "--high-pass", dest="hipass", type="float", default=1,
                      help="alpha (between zero and one) for a high-pass filter")
    parser.add_option("-d", "--dist", dest="dist", default=True, type="int",
                      help="distortion level (0 to 255)")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        sys.exit()
    
    infile = open(args[0], "rb")
    
    hdr = infile.read(0x2c)
    if len(hdr) < 0x2c:
        print "file is too short"
        infile.close()
        sys.exit()
    
    WaveHdr = namedtuple('WaveHdr', 'tag total_len fmt_tag bits a b rate c d e data_tag data_len')
    unpacked_hdr = WaveHdr._make(struct.unpack("<4sI8sIHHIIHH4sI", hdr))
    
    data_len = unpacked_hdr.data_len
    left_average_list = []
    right_average_list = []
    left_energy_list = []
    right_energy_list = []
    
    NUM_SAMPLES = 256
    
    while data_len >= NUM_SAMPLES*2*2:
        curr_buff = infile.read(NUM_SAMPLES*2*2)
        data_len -= len(curr_buff)
        
        # turn into numbers, separating left and right
        curr_buff_left = list(struct.unpack("<" + "hxx"*NUM_SAMPLES, curr_buff))
        curr_buff_right = list(struct.unpack("<" + "xxh"*NUM_SAMPLES, curr_buff))
        
        left_average = compute_average(curr_buff_left)
        right_average = compute_average(curr_buff_right)
        
        left_average_list.append(left_average)
        right_average_list.append(right_average)
        
        left_energy_list.append(compute_energy(curr_buff_left, left_average))
        right_energy_list.append(compute_energy(curr_buff_right, right_average))
    
    # report
    print "left average:", compute_average(left_average_list), "right average:", compute_average(right_average_list)
    print "left energy:", reduce(lambda x,y: x+y, left_energy_list, 0), "right energy:", reduce(lambda x,y: x+y, right_energy_list, 0)
    
    infile.close()
