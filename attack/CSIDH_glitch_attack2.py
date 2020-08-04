import CSIDH_glitch
import sys


def main(argv):
    if(len(argv) != 1):
        print("\nplease specify the number of trials");
        print("python CSIDH_glitch_attack2.py [ntrials]")
        print("e.g. python CSIDH_glitch_attack2.py 100")
        sys.exit()
    else:
        CSIDH_glitch.main(["./", "ATTACK2_D", argv[0]])



if __name__ == "__main__":
    main(sys.argv[1:])
