#!/usr/bin/python
import struct, sys, math, numpy, functools
from scipy.fft import fft, ifft
from optparse import OptionParser
from collections import namedtuple
import matplotlib.pyplot as plt


class OctaverContext:
    def __init__(self, sample_cnt, bits, scale_factor):
        # sanity checks
        if bits not in [8,16,24,32]: raise ValueError()
        if sample_cnt not in [128,256,512,1024,2048]: raise ValueError()
        if scale_factor > 2 or scale_factor < 0.5: raise ValueError()
        
        # sizes and lengths
        self.sample_cnt = sample_cnt
        self.bits = bits
        self.max_sample = 2**(bits-1)
        self.scale_factor = scale_factor
        self.frame_idx = 0
        
        #
        # save the last phase shift of the output signal, in order to make the phase continuous
        # this is saved per frequency bin
        #
        self.output_phase_shifts = numpy.array([1] * sample_cnt, dtype = complex)
        
        # window manipulations for smoother signals
        self.hann_window = numpy.array([math.sin(math.pi*n/sample_cnt)**2 for n in range(sample_cnt)])
        self.hann_window_scaled = \
            numpy.array([math.sin(math.pi * n / sample_cnt / scale_factor)**2 \
                for n in range(math.ceil(sample_cnt * scale_factor))])
        
        # context variables for each window: previous clean buffer, previous four modified buffers
        self.prev_buff = numpy.array([0] * sample_cnt, dtype = complex)
        self.prev_mod_buffs = [numpy.array([0] * int(math.ceil(sample_cnt * self.scale_factor)), dtype = complex) for i in range(4)]
    
    def extrapolate(buff, extrapolation_factor):
        new_size = math.ceil(buff.size * extrapolation_factor)
        
        # result is a bigger array, according to the extrapolation factor
        res = numpy.array([0] * new_size, dtype=complex)
        
        # we iterate over the indices, using a linear extrapolation
        idx_inc = 1 / extrapolation_factor
        idx_curr = 0
        
        for idx in range(new_size):
            target_idx = idx * idx_inc
            if target_idx % 1 == 0:
                res[idx] = buff[int(target_idx)]
            elif int(target_idx) >= buff.size - 1:
                res[idx] = buff[buff.size - 1]
            else:
                low_num = buff[int(target_idx)]
                high_num = buff[int(target_idx) + 1]
                frac = target_idx % 1
                res[idx] = low_num * (1 - frac) + high_num * frac
        
        return res
    
    def get_windowed_quarters(self, buff, start_quarter):
        #
        # since the buffers are shifted by half the sampling window, we want to pad them in order to be able to take
        # just the quarters we need from them
        #
        l = [0] * int(self.sample_cnt // 2) # half-window padding
        l += list(buff * self.hann_window_scaled) # scaled sample
        l += [0] * (self.sample_cnt * 2 - self.hann_window_scaled.size) # padding to a double window
        l += [0] * int(self.sample_cnt // 2) # padding of another half-window
        l = numpy.array(l)
        
        # get the two relevant half-window chunks to form a window-sized buffer
        return l[(start_quarter + 1) * int(self.sample_cnt // 2) : (start_quarter + 3) * int(self.sample_cnt // 2)]
    
    def process_buff(self, curr_buff):
        # start with a copy of the new buffer, in the form of numpy.ndarray
        new_buff = numpy.array(list(curr_buff), dtype = complex)
        
        # convert current sample to a complex number with real part in the range [-1,1]
        new_buff = new_buff / self.max_sample
                
        # create the mid-buffer, composed of the last half of the previous buffer and the first half of the new buffer
        mid_buff = numpy.array(self.prev_buff[self.sample_cnt//2:].tolist() + new_buff[:self.sample_cnt//2].tolist())
        
        # compute the FFT of both the new buffer and the middle buffer, to get the phase shifts
        mid_buff_fft = fft(mid_buff * self.hann_window)
        new_buff_fft = fft(new_buff * self.hann_window)
        
        # compute the phase shift of each (non-DC) frequency bin and align the output phase accordingly
        for k in range(1, self.sample_cnt):
            phase_shift = numpy.exp(1j * numpy.pi / self.scale_factor * k)
            mid_buff_fft[k] *= self.output_phase_shifts[k] * phase_shift
            if k % 2 != 0:
                mid_buff_fft[k] *= -1
            new_buff_fft[k] *= self.output_phase_shifts[k] * (phase_shift**2)
            self.output_phase_shifts[k] *= (phase_shift**2)
        
        # stretch the arrays
        new_mod_buff = OctaverContext.extrapolate(ifft(new_buff_fft), self.scale_factor)
        mid_mod_buff = OctaverContext.extrapolate(ifft(mid_buff_fft), self.scale_factor)
        
        # if self.frame_idx < 10:
        #     plt.plot(list(range(self.sample_cnt)), self.get_windowed_quarters(self.prev_mod_buffs[0], 3))
        #     plt.plot(list(range(self.sample_cnt)), self.get_windowed_quarters(self.prev_mod_buffs[1], 2))
        #     plt.plot(list(range(self.sample_cnt)), self.get_windowed_quarters(self.prev_mod_buffs[2], 1))
        #     plt.plot(list(range(self.sample_cnt)), self.get_windowed_quarters(self.prev_mod_buffs[3], 0))
        #     plt.plot(list(range(self.sample_cnt)), self.get_windowed_quarters(mid_mod_buff, -1))
        #     plt.grid()
        #     plt.show()
        
        # combine the different windows from each array
        res = self.get_windowed_quarters(self.prev_mod_buffs[0], 3)
        res += self.get_windowed_quarters(self.prev_mod_buffs[1], 2)
        res += self.get_windowed_quarters(self.prev_mod_buffs[2], 1)
        res += self.get_windowed_quarters(self.prev_mod_buffs[3], 0)
        res += self.get_windowed_quarters(mid_mod_buff, -1)
        
        # prepare for the next window
        self.prev_buff = new_buff
        self.prev_mod_buffs[0] = self.prev_mod_buffs[2]
        self.prev_mod_buffs[1] = self.prev_mod_buffs[3]
        self.prev_mod_buffs[2] = mid_mod_buff
        self.prev_mod_buffs[3] = new_mod_buff
        self.frame_idx += 1
        
        return list(map(lambda x: int(x.real), res * self.max_sample))


if __name__ == "__main__":
    parser = OptionParser(usage="usage: %prog [options] WAVFILE")
    parser.add_option("-o", "--out", dest="outfile", help="output filename")
    parser.add_option("-b", "--buff-size", dest="buff_size", type=int, default=1024, help="buffer size (default: 1024)")
    parser.add_option("-p", "--plot-frames", dest="plot_frames", type=int, default=0, help="plot the first frames (default: 0)")
    parser.add_option("-s", "--semitones", dest="semitones", type=int, default=12, help="semitones for pitch shift (default: 12)")

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
    
    scale_factor = 2**(options.semitones / 12)
    contexts = [OctaverContext(options.buff_size, unpacked_hdr.bits, scale_factor),
                OctaverContext(options.buff_size, unpacked_hdr.bits, scale_factor)]
    
    data_len = unpacked_hdr.data_len
    
    frame_idx = 0
    
    while data_len >= options.buff_size*2*unpacked_hdr.ch:
        curr_buff = infile.read(options.buff_size*2*unpacked_hdr.ch)
        data_len -= len(curr_buff)
        
        # turn into numbers, separating left and right if needed
        if unpacked_hdr.ch == 1:
            curr_buff_left = list(struct.unpack(
                "<" + "h"*options.buff_size, curr_buff
            ))
            curr_buff_right = [0] * options.buff_size
        else:
            curr_buff_left = list(struct.unpack(
                "<" + "hxx"*options.buff_size, curr_buff
            ))
            curr_buff_right = list(struct.unpack(
                "<" + "xxh"*options.buff_size, curr_buff
            ))
        mod_buff_left = contexts[0].process_buff(curr_buff_left)
        if unpacked_hdr.ch > 1:
            mod_buff_right = contexts[1].process_buff(curr_buff_right)
        else:
            mod_buff_right = [0] * options.buff_size
        
        if unpacked_hdr.ch > 1:
            # interleave the modified buffers and write them to the output file
            mod_buff = zip(mod_buff_left, mod_buff_right)
            mod_buff = map(list, mod_buff)
            mod_buff = functools.reduce(lambda x,y: x+y, mod_buff, [])
        else:
            mod_buff = mod_buff_left
        
        # prevent clipping and align the samples in case there's an overflow/underflow
        mod_buff = list(map(lambda x: max(min(x, 2**(unpacked_hdr.bits-1) - 1), -2**(unpacked_hdr.bits-1)), mod_buff))

        if unpacked_hdr.ch == 1:
            mod_buff = struct.pack("<" + "h"*options.buff_size, *mod_buff)
        else:
            mod_buff = struct.pack("<" + "hh"*options.buff_size, *mod_buff)
        outfile.write(mod_buff)
        
        if frame_idx < options.plot_frames:
            plt.plot(list(range(options.buff_size)), curr_buff_left)
            plt.plot(list(range(options.buff_size)), mod_buff_left)
            plt.grid()
            plt.show()
        frame_idx += 1
    
    # handle the remainder
    outfile.write(infile.read())
    outfile.close()
    infile.close()
