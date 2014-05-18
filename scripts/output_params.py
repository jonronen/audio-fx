import math

if __name__ == "__main__":
    out_str = "static const unsigned short g_sine_phase[] = {"
    for i in range(256):
        if (i % 8) == 0: out_str += "\n   "
        if (i % 4) == 0: out_str += " "
        out_str += " 0x%04x," % (0x1000-((256-i)**3)/4096)
    out_str += "\n};\n"
    print out_str
    
    out_str = "static const unsigned short g_exp_phase[] = {"
    for i in range(256):
        if (i % 8) == 0: out_str += "\n   "
        if (i % 4) == 0: out_str += " "
        out_str += " 0x%04x," % (int(1.033026 ** i) -1)
    out_str += "\n};\n"
    print out_str

