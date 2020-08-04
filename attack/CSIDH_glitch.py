import random
from scipy import signal
import matplotlib.pyplot as plt
import numpy as np
import time
import chipwhisperer as cw
import sys
import datetime
import os
import csv

SCOPETYPE = 'OPENADC'
PLATFORM = 'CWLITEARM'

CRYPTO_TARGET = "NONE"

GLITCH = True

BIN = 'speed-CWLITEARM.hex'

def setGlitch(scope):
    # The clock signal that the glitch DCM is using as input.
    # This DCM can be clocked from two different sources:
    # “target”: The HS1 clock from the target device
    # “clkgen”: The CLKGEN DCM output            
    scope.glitch.clk_src = "clkgen"

    # The type of output produced by the glitch module.
    # There are 5 ways that the glitch module can combine the clock with its glitch pulses:
    # “clock_only”: Output only the original input clock.
    # “glitch_only”: Output only the glitch pulses - do not use the clock.
    # “clock_or”: Output is high if either the clock or glitch are high.
    # “clock_xor”: Output is high if clock and glitch are different.
    # “enable_only”: Output is high for glitch.repeat cycles.
    # Some of these settings are only useful in certain scenarios:
    # Clock glitching: “clock_or” or “clock_xor”
    # Voltage glitching: “glitch_only” or “enable_only”
    scope.glitch.output = "clock_xor"

    # The trigger signal for the glitch pulses.
    # The glitch module can use four different types of triggers:
    # “continuous”: Constantly trigger glitches
    # “manual”: Only trigger glitches through API calls/GUI actions
    # “ext_single”: Use the trigger module. One glitch per scope arm.
    # “ext_continuous”: Use the trigger module. Many glitches per arm.            
    scope.glitch.trigger_src = "ext_single"

    # The width of a single glitch pulse, as a percentage of one period.
    # One pulse can range from -49.8% to roughly 49.8% of a period.
    # The system may not be reliable at 0%. Note that negative widths are allowed;
    # these act as if they are positive widths on the other half of the clock cycle.
    scope.glitch.width = -9


    # The offset from a rising clock edge to a glitch pulse rising edge, 
    # as a percentage of one period.
    # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
    # allowing glitches to be swept over the entire clock cycle.
    scope.glitch.offset = -38.3


    # The clock signal routed to the HS2 high speed output pin.
    # Allowed clock signals are:
    # “clkgen”: The output from the CLKGEN module
    # “glitch”: The output from the glitch module
    # “disabled” / None: No clock; output driven low
    scope.io.hs2 = "glitch"

def setDefault(scope):
    scope.glitch.output = "clock_only"
    scope.io.hs2 = "clkgen"

def main(argv):
    if(len(argv) != 3):
        print(
            "\nplease specify the path, type and numer of trials\npython CSIDH_glitch.py [path] [type] [ntrials]\ne.g. python CSIDH_glitch.py /home/me/chipwhisperer/hardware/victims/firmware/csidh_trace/ ATTACK1 100")
        sys.exit()
    else:
        random.seed(13)
        PATH = argv[0]
        TYPE = argv[1]          # ATTACK1 or ATTACK2
        NTRIAL = int(argv[2])
        FULLPATH = PATH + BIN

        # Setup_Generic ##################################################################
        scope = cw.scope()
        target = cw.target(scope)

        if "STM" in PLATFORM or PLATFORM == "CWLITEARM" or PLATFORM == "CWNANO":
            prog = cw.programmers.STM32FProgrammer
        elif PLATFORM == "CW303" or PLATFORM == "CWLITEXMEGA":
            prog = cw.programmers.XMEGAProgrammer
        else:
            prog = None

        import time
        time.sleep(0.05)
        scope.default_setup()
        

        def reset_target(scope):
            if PLATFORM == "CW303" or PLATFORM == "CWLITEXMEGA":
                scope.io.pdic = 'low'
                time.sleep(0.05)
                scope.io.pdic = 'high_z'  # XMEGA doesn't like pdic driven high
                time.sleep(0.05)
            else:
                scope.io.nrst = 'low'
                time.sleep(0.05)
                scope.io.nrst = 'high'
                time.sleep(0.05)
        # Setup_Generic ##################################################################

        # create firmware
        os.chdir(PATH)
        #os.system('make clean PLATFORM=CWLITEARM CRYPTO_TARGET=NONE')
        #os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=TEST OPT=0")
        
        if((TYPE == "ATTACK1") or (TYPE == "ATTACK1_D")):
            if(TYPE == "ATTACK1"):
                os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK1 OPT=3")
            else:
                os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK1_D OPT=3")
                #os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK1_D OPT=3 TYPE=CM")

            # Sets up sane capture defaults for this scope
            # 45dB gain
            # 5000 capture samples
            # 0 sample offset
            # rising edge trigger
            # 7.37MHz clock output on hs2
            # 4*7.37MHz ADC clock
            # tio1 = serial rx
            # tio2 = serial tx
            scope.default_setup()

            scope.clock.adc_src = 'clkgen_x1'

            fw_path = FULLPATH.format(PLATFORM)
            print(fw_path)

            reset_target(scope)
            target.flush()

            cw.program_target(scope, prog, fw_path)

            glitches = []
            glitches.append(["[scope.glitch.width", "scope.glitch.offset", "scope.glitch.repeat", "scope.glitch.ext_offset", "good/bad/crash]"])

            if(GLITCH) :
                setGlitch(scope)

            # The response took 1086919328 cycles
            # -O3
            ext_offset = 222534093           

            wrong = 0
            crashes = 0
            for i in range(0, NTRIAL):
                if(GLITCH) :
                    # scope.glitch.width = random.randint(-49, 49)

                    # scope.glitch.offset = random.randint(-49, 49)

                    # The width of a single glitch pulse, as a percentage of one period.
                    # One pulse can range from -49.8% to roughly 49.8% of a period.
                    # The system may not be reliable at 0%. Note that negative widths are allowed;
                    # these act as if they are positive widths on the other half of the clock cycle.
                    # scope.glitch.width = random.randint(-49, 49)
                    scope.glitch.width = -9
                    
                    # The offset from a rising clock edge to a glitch pulse rising edge, 
                    # as a percentage of one period.
                    # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
                    # allowing glitches to be swept over the entire clock cycle.                        
                    # scope.glitch.offset = random.randint(-49, 49)
                    scope.glitch.offset = -38.3                    

                    # How long the glitch module waits between a trigger and a glitch.
                    # After the glitch module is triggered, it waits for a number of clock cycles 
                    # before generating glitch pulses. This delay allows the glitch to be inserted 
                    # at a precise moment during the target’s execution to glitch specific instructions.
                    scope.glitch.ext_offset = random.randint(0, ext_offset)
                    # scope.glitch.ext_offset = 108466842
                    
                    # The number of glitch pulses to generate per trigger.
                    # When the glitch module is triggered, it produces a number of pulses 
                    # that can be combined with the clock signal. This setting allows for 
                    # the glitch module to produce stronger glitches (especially during voltage glitching).
                    # Repeat counter must be in the range [1, 255].
                    # scope.glitch.repeat = random.randint(1, 4)
                    scope.glitch.repeat = 3
                    
                reset_target(scope)
                target.flush()


                print("\n--------------------------------\nstarting capture " + str(i) + " : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                #scope.adc.timeout = 210
                scope.arm()
                #scope.adc.timeout = 210
                target.write("s")
                time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                time.sleep(40)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                print("<output>\n" + response + "\n</output>")

                if(GLITCH) :
                    if "good" not in response:
                        # Something abnormal happened
                        if len(response) > 0:
                            # Possible glitch!
                            print("\nBad shared secret\nGlitch at offset {} and repeat = {}".format(scope.glitch.ext_offset, scope.glitch.repeat))
                            wrong = wrong + 1
                            glitches.append([scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "bad"])

                        else:
                            # Crash, reset and try again
                            print("\nProbably crashed at {} and repeat = {}".format(scope.glitch.ext_offset, scope.glitch.repeat))
                            crashes = crashes + 1
                            glitches.append([scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "crash"])
                    else:
                        print("\nGood shared secret\nGlitch at offset {} and repeat = {}".format(scope.glitch.ext_offset, scope.glitch.repeat))
                        glitches.append([scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "good"])



                print("end of capture " + str(i) + " : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

            glitches.append(["-----------------------------"])
            glitches.append(["ntrial, wrong, crashes"])
            glitches.append([NTRIAL, wrong, crashes])

            if(TYPE == "ATTACK1"):
                PATH = "attack1_" + datetime.datetime.now().strftime("%d%m%Y_%H%M") + ".csv"
            else:
                PATH = "attack1_D_" + datetime.datetime.now().strftime("%d%m%Y_%H%M") + ".csv"

            # save raw data as csv
            with open(PATH, mode='w') as csv_file:
                csv_writer = csv.writer(csv_file)
                for entry in glitches:
                    csv_writer.writerow(entry)


        elif(TYPE == "ATTACK2"):
            os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK2 OPT=3")

            # Sets up sane capture defaults for this scope
            # 45dB gain
            # 5000 capture samples
            # 0 sample offset
            # rising edge trigger
            # 7.37MHz clock output on hs2
            # 4*7.37MHz ADC clock
            # tio1 = serial rx
            # tio2 = serial tx
            scope.default_setup()

            scope.clock.adc_src = 'clkgen_x1'

            fw_path = FULLPATH.format(PLATFORM)
            print(fw_path)

            reset_target(scope)
            target.flush()

            cw.program_target(scope, prog, fw_path)

            glitches = []
            glitches.append(["[isogeny", "scope.glitch.width", "scope.glitch.offset", "scope.glitch.repeat", "scope.glitch.ext_offset", "good/bad/crash]"])

            # The response took 4846257 cycles : 1. isogeny computation
            # -O0
            # ext_offset = 4846257
            # -O3
            ext_offset = 83289

            wrong = 0
            crashes = 0

            setDefault(scope)

            for i in range(0, NTRIAL):

                if(GLITCH) :
                    #isogeny = random.randint(1, 2)
                    isogeny = 2

                    if(isogeny == 1):
                        setGlitch(scope)

                        # The width of a single glitch pulse, as a percentage of one period.
                        # One pulse can range from -49.8% to roughly 49.8% of a period.
                        # The system may not be reliable at 0%. Note that negative widths are allowed;
                        # these act as if they are positive widths on the other half of the clock cycle.
                        # scope.glitch.width = random.randint(-49, 49)
                        
                        # The offset from a rising clock edge to a glitch pulse rising edge, 
                        # as a percentage of one period.
                        # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
                        # allowing glitches to be swept over the entire clock cycle.                        
                        # scope.glitch.offset = random.randint(-49, 49)

                        # How long the glitch module waits between a trigger and a glitch.
                        # After the glitch module is triggered, it waits for a number of clock cycles 
                        # before generating glitch pulses. This delay allows the glitch to be inserted 
                        # at a precise moment during the target’s execution to glitch specific instructions.
                        scope.glitch.ext_offset = random.randint(0, ext_offset)
                        # scope.glitch.ext_offset = 108466842
                        
                        # The number of glitch pulses to generate per trigger.
                        # When the glitch module is triggered, it produces a number of pulses 
                        # that can be combined with the clock signal. This setting allows for 
                        # the glitch module to produce stronger glitches (especially during voltage glitching).
                        # Repeat counter must be in the range [1, 255].
                        scope.glitch.repeat = random.randint(1, 4)
                        #scope.glitch.repeat = 1
                    
                reset_target(scope)
                target.flush()


                print("\n--------------------------------\nstarting capture " + str(i) + "\n--------------------------------\ncapture of 1. isogeny  : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 180
                scope.arm()
                scope.adc.timeout = 180
                target.write("s")
                time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                #time.sleep(60)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                print("<output>\n" + response + "\n</output>")

                print("end of 1. isogeny : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                setDefault(scope)

                # The response took 3436390 cycles : 2. isogeny computation
                # -O0
                # ext_offset = 3436390
                # -O3
                ext_offset = 8580178
                             
                
                
                if(GLITCH) :

                    if(isogeny == 2):
                        setGlitch(scope)

                        # The width of a single glitch pulse, as a percentage of one period.
                        # One pulse can range from -49.8% to roughly 49.8% of a period.
                        # The system may not be reliable at 0%. Note that negative widths are allowed;
                        # these act as if they are positive widths on the other half of the clock cycle.
                        # scope.glitch.width = random.randint(-49, 49)
                        
                        # The offset from a rising clock edge to a glitch pulse rising edge, 
                        # as a percentage of one period.
                        # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
                        # allowing glitches to be swept over the entire clock cycle.                        
                        # scope.glitch.offset = random.randint(-49, 49)

                        # How long the glitch module waits between a trigger and a glitch.
                        # After the glitch module is triggered, it waits for a number of clock cycles 
                        # before generating glitch pulses. This delay allows the glitch to be inserted 
                        # at a precise moment during the target’s execution to glitch specific instructions.
                        scope.glitch.ext_offset = random.randint(1, ext_offset)
                        # scope.glitch.ext_offset = 108466842
                        
                        # The number of glitch pulses to generate per trigger.
                        # When the glitch module is triggered, it produces a number of pulses 
                        # that can be combined with the clock signal. This setting allows for 
                        # the glitch module to produce stronger glitches (especially during voltage glitching).
                        # Repeat counter must be in the range [1, 255].
                        scope.glitch.repeat = random.randint(1, 4)
                        #scope.glitch.repeat = 1


                print("\n--------------------------------\ncapture of 2. isogeny : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 60
                scope.arm()
                scope.adc.timeout = 60                
                target.write("n")
                time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                #time.sleep(10)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                print("<output>\n" + response + "\n</output>")


                print("end of 2. isogeny : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                setDefault(scope)

                print("\n--------------------------------\nstarting capture of result : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 30
                scope.arm()
                scope.adc.timeout = 30                
                target.write("n")
                time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                #time.sleep(10)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                print("<output>\n" + response + "\n</output>")


                print("end of capture " + str(i) + " : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                if(GLITCH) :
                    if "good" not in response:
                        # Something abnormal happened
                        if len(response) > 0:
                            # Possible glitch!
                            print("\nBad shared secret\nGlitching isogeny {} at offset {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                            wrong = wrong + 1
                            glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "bad"])

                        else:
                            # Crash, reset and try again
                            print("\nProbably crashed at isogeny {} at {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                            crashes = crashes + 1
                            glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "crash"])
                    else:
                        print("\nGood shared secret\nGlitching isogeny {} at offset {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                        glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "good"])


            glitches.append(["-----------------------------"])
            glitches.append(["ntrial, wrong, crashes"])
            glitches.append([NTRIAL, wrong, crashes])


            PATH = "attack2_" + datetime.datetime.now().strftime("%d%m%Y_%H%M") + ".csv"

            # save raw data as csv
            with open(PATH, mode='w') as csv_file:
                csv_writer = csv.writer(csv_file)
                for entry in glitches:
                    csv_writer.writerow(entry)


        elif(TYPE == "ATTACK2_D"):
            #os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK2_D OPT=2 TYPE=CM")
            os.system("make PLATFORM=CWLITEARM CRYPTO_TARGET=NONE FUNC_SEL=ATTACK2_D OPT=3")

            # Sets up sane capture defaults for this scope
            # 45dB gain
            # 5000 capture samples
            # 0 sample offset
            # rising edge trigger
            # 7.37MHz clock output on hs2
            # 4*7.37MHz ADC clock
            # tio1 = serial rx
            # tio2 = serial tx
            scope.default_setup()

            scope.clock.adc_src = 'clkgen_x1'

            fw_path = FULLPATH.format(PLATFORM)
            print(fw_path)

            reset_target(scope)
            target.flush()

            cw.program_target(scope, prog, fw_path)

            glitches = []
            glitches.append(["[isogeny", "scope.glitch.width", "scope.glitch.offset", "scope.glitch.repeat", "scope.glitch.ext_offset", "good/bad/crash]"])


            # The response took 4846257 cycles : 1. isogeny computation
            # -O0
            # ext_offset = 4846257
            # -O3
            ext_offset = 90082

            wrong = 0
            crashes = 0

            setDefault(scope)
      
            
            test = 5639046 - 10
            width = -49
            for i in range(0, NTRIAL):
                test = test + 1 
                width  = width + 3

                reset_target(scope)
                target.flush()

                # The response took 4846257 cycles : 1. isogeny computation
                # -O0
                # ext_offset = 4846257
                # -O3
                ext_offset = 90082

                if(GLITCH) :
                    #isogeny = random.randint(1, 2)
                    isogeny = 1

                    if(isogeny == 1):
                        setGlitch(scope)

                        # The width of a single glitch pulse, as a percentage of one period.
                        # One pulse can range from -49.firmware/csidh/attack2_D_14052020_2028_m1_1.csv8% to roughly 49.8% of a period.
                        # The system may not be reliable at 0%. Note that negative widths are allowed;
                        # these act as if they are positive widths on the other half of the clock cycle.
                        # scope.glitch.width = random.randint(-49, 49)
                        scope.glitch.width = -9
                        
                        # The offset from a rising clock edge to a glitch pulse rising edge, 
                        # as a percentage of one period.
                        # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
                        # allowing glitches to be swept over the entire clock cycle.                        
                        # scope.glitch.offset = random.randint(-49, 49)
                        scope.glitch.offset = -38.3

                        # How long the glitch module waits between a trigger and a glitch.
                        # After the glitch module is triggered, it waits for a number of clock cycles 
                        # before generating glitch pulses. This delay allows the glitch to be inserted 
                        # at a precise moment during the target’s execution to glitch specific instructions.
                        scope.glitch.ext_offset = random.randint(1, ext_offset)
                        #scope.glitch.ext_offset = 57248
                        
                        # The number of glitch pulses to generate per trigger.
                        # When the glitch module is triggered, it produces a number of pulses 
                        # that can be combined with the clock signal. This setting allows for 
                        # the glitch module to produce stronger glitches (especially during voltage glitching).
                        # Repeat counter must be in the range [1, 255].
                        #scope.glitch.repeat = random.randint(1, 5)
                        scope.glitch.repeat = 3
                    

                print("\n--------------------------------\nstarting capture " + str(i) + "\n--------------------------------\ncapture of 1. isogeny  : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 10
                scope.arm()
                scope.adc.timeout = 10
                target.write("s")
                #time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                #time.sleep(5)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                #print("<output>\n" + response + "\n</output>")

                print("end of 1. isogeny : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                setDefault(scope)

                # The response took 3436390 cycles : 2. isogeny computation
                # -O0
                # response_time = 3436390
                # -O380392
                ext_offset = 60817547

                if(GLITCH) :

                    if(isogeny == 2):
                        setGlitch(scope)

                        # The width of a single glitch pulse, as a percentage of one period.
                        # One pulse can range from -49.8% to roughly 49.8% of a period.
                        # The system may not be reliable at 0%. Note that negative widths are allowed;
                        # these act as if they are positive widths on the other half of the clock cycle.
                        # scope.glitch.width = random.randint(-49, 49)
                        scope.glitch.width = -9
                        
                        # The offset from a rising clock edge to a glitch pulse rising edge, 
                        # as a percentage of one period.
                        # A pulse may begin anywhere from -49.8% to 49.8% away from a rising edge, 
                        # allowing glitches to be swept over the entire clock cycle.                        
                        # scope.glitch.offset = random.randint(-49, 49)
                        scope.glitch.offset =  -38.3

                        # How long the glitch module waits between a trigger and a glitch.
                        # After the glitch module is triggered, it waits for a number of clock cycles 
                        # before generating glitch pulses. This delay allows the glitch to be inserted 
                        # at a precise moment during the target’s execution to glitch specific instructions.
                        scope.glitch.ext_offset = random.randint(1, ext_offset)
                        #scope.glitch.ext_offset = 5393209 + (i*2)
                        #scope.glitch.ext_offset = 8050242

                        
                        # The number of glitch pulses to generate per trigger.
                        # When the glitch module is triggered, it produces a number of pulses 
                        # that can be combined with the clock signal. This setting allows for 
                        # the glitch module to produce stronger glitches (especially during voltage glitching).
                        # Repeat counter must be in the range [1, 255].
                        #scope.glitch.repeat = random.randint(1, 5)
                        scope.glitch.repeat = 3
                        #scope.glitch.repeat = 1


                print("\n--------------------------------\ncapture of 2. isogeny : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 10
                scope.arm()
                scope.adc.timeout = 10                
                target.write("n")
                #time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                time.sleep(1)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                #print("<output>\n" + response + "\n</output>")


                print("end of 2. isogeny : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                setDefault(scope)

                print("\n--------------------------------\nstarting capture of result : " +
                        datetime.datetime.now().strftime("%H:%M:%S"))
                scope.adc.timeout = 10
                scope.arm()
                scope.adc.timeout = 10               
                target.write("n")
                time.sleep(0.1)

                ret = scope.capture()
                if ret:
                    print('Timeout happened during acquisition')

                #time.sleep(10)

                if SCOPETYPE == "OPENADC":
                    response_time = scope.adc.trig_count
                    print("The response took {} cycles".format(response_time))
                response = target.read(timeout=0)
                print("<output>\n" + response + "\n</output>")


                print("end of capture " + str(i) + " : " + datetime.datetime.now().strftime("%H:%M:%S") +
                        "\n--------------------------------")

                if(GLITCH) :
                    if "good" not in response:
                        # Something abnormal happened
                        if len(response) > 0:
                            # Possible glitch!
                            print("\nBad shared secret\nGlitching isogeny {} at offset {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                            wrong = wrong + 1
                            glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "bad"])

                        else:
                            # Crash, reset and try again
                            print("\nProbably crashed at isogeny {} at {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                            crashes = crashes + 1
                            glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "crash"])
                    else:
                        print("\nGood shared secret\nGlitching isogeny {} at offset {} and repeat = {}".format(isogeny, scope.glitch.ext_offset, scope.glitch.repeat))
                        glitches.append([isogeny, scope.glitch.width, scope.glitch.offset, scope.glitch.repeat, scope.glitch.ext_offset, "good"])


            glitches.append(["-----------------------------"])
            glitches.append(["ntrial, wrong, crashes"])
            glitches.append([NTRIAL, wrong, crashes])


            PATH = "attack2_D_" + datetime.datetime.now().strftime("%d%m%Y_%H%M") + ".csv"

            # save raw data as csv
            with open(PATH, mode='w') as csv_file:
                csv_writer = csv.writer(csv_file)
                for entry in glitches:
                    csv_writer.writerow(entry)


        scope.dis()
        target.dis()


if __name__ == "__main__":
    main(sys.argv[1:])

