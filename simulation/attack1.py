import math, random
import copy

# odd primes
ls = [359, 353, 349, 347, 337, 331, 317, 313, 311,
      307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
      227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
      149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
      71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3,
      587, 373, 367]
n = len(ls)

# p of CSIDH-512
p = 4
for l in ls:
    p *= l
p -= 1

# the cost ratios of operations (determined for the STM32F303/ChipWhisperer Target)
rM = 1.0  # multiplications
rS = 0.99949889613
ra = 0.02626500462

transcript = []
totalcost = 0
isog_count = 0
real_isog = 0
dummy_isog = 0

# the cost of calculating :
# (8t-4)M + (4t-2)S + (8t-6)a, where t is the bit length of m
def IsogenyAddition(l):
    if(l==3):
        t = 4*rM+2*rS+6*ra
    else:
        t = 2*(4*rM+2*rS+6*ra)
    return t

# the cost of multiplication by m:
# (8t-4)M + (4t-2)S + (8t-6)a, where t is the bit length of m
def Ladder(m):
    t = int(math.log(m, 2))
    return (8*t-4)*rM + (4*t-2)*rS + (8*t-6)*ra

# the cost of generating a kernel set from a kernel generator of order l:
# 4(d-1)M + 2(d-1)S + 2(3d-3)a, where l = 2d + 1
def KernelPoints(l):
    d = int((l-1)/2)
    return 4*(d-1)*rM +  2*(d-1)*rS + 2*(3*d-4)*ra
    #return 4*(d-1)*rM +  2*(d-1)*rS + 2*(3*d-3)*ra

# the cost of evaluating a point under an isogeny of degree l:
# 4dM + 2S + 2(d+1)a, where l = 2d + 1
def IsogenyPoint(l):
    d = int((l-1)/2)
    return 4*d*rM + 2*rS + 2*(d+1)*ra

# the cost of calculating the domain of an isogeny of degree l:
# (2d+t)M + (2t+6)S + 6a, where l = 2d + 1, t is the bit length of l
def IsogenyCurve(l):
    t = int(math.log(l, 2))
    d = int((l-1)/2)
    return (2*d+t)*rM + (2*t+6)*rS + 6*ra

# the cost of calculating the domain of an isogeny of degree l:
# (2d+t)M + (2t+6)S + 6a, where l = 2d + 1, t is the bit length of l
def Elligator():
    return (260)*rM + (516)*rS + 6*ra


# fault injection on a set S of indices
# params:
#   e: secret key (remaining real isogenies per order)
#   ed: remaining dummy per order
#   S: index set for current batch
def action_for_S(e, ed, S):
    global totalcost
    global transcript
    global isog_count
    global dummy_isog
    global real_isog
    k = 1

    totalcost = totalcost + Elligator()
    transcript.append(['Elligator', totalcost])

    for i in S:
        k *= ls[i]

    # [(p+1)/k]P_0
    totalcost = totalcost + Ladder((p + 1)/k)
    transcript.append(['Ladder+', totalcost])

    # [(p+1)/k]P_1
    totalcost = totalcost + Ladder((p + 1)/k)
    transcript.append(['Ladder-', totalcost])


    for i in S:
        s = int(e[i] < 0) # the sign bit of ei

        # [k/li]P_s
        totalcost = totalcost + Ladder(k/ls[i])
        if(s == 0):
            transcript.append(['Ladder+', totalcost])
        else:
            transcript.append(['Ladder-', totalcost])

        # [li]P_{1-s}
        totalcost = totalcost + Ladder(ls[i])
        if(s == 0):
            transcript.append(['Ladder-', totalcost])
        else:
            transcript.append(['Ladder+', totalcost])


        if not random.randint(1, ls[i]) == ls[i]:   # if Q neq infty
            isog_count = isog_count + 1
            if not e[i] == 0:
                # true isogeny
                real_isog = real_isog + 1
                # generating kernel points
                totalcost = totalcost + KernelPoints(ls[i])
                if(s == 0):
                    transcript.append(['real KernelPoints+', totalcost])
                else:
                    transcript.append(['real KernelPoints-', totalcost])

                if i not in [71,72,73]:
                    # phi(P_0)
                    totalcost = totalcost + IsogenyPoint(ls[i])
                    if(s == 0):
                        transcript.append(['real IsogenyPoint+', totalcost])
                    else:
                        transcript.append(['real IsogenyPoint-', totalcost])

                    # phi(P_1)
                    totalcost = totalcost + IsogenyPoint(ls[i])
                    if(s == 0):
                        transcript.append(['real IsogenyPoint-', totalcost])
                    else:
                        transcript.append(['real IsogenyPoint+', totalcost])

                # codomain
                totalcost = totalcost + IsogenyCurve(ls[i])
                if(s == 0):
                    transcript.append(['real IsogenyCurve+', totalcost])
                else:
                    transcript.append(['real IsogenyCurve-', totalcost])

                totalcost = totalcost + IsogenyAddition(ls[i])
                if(s == 0):
                    transcript.append(['real IsogenyAddition+', totalcost])
                else:
                    transcript.append(['real IsogenyAddition-', totalcost])

                if(s == 0):
                    e[i] -= 1
                else:
                    e[i] += 1
            else:
                # dummy isogeny
                dummy_isog = dummy_isog + 1
                # [l_i]P_s
                totalcost = totalcost + KernelPoints(ls[i])
                if(s == 0):
                    transcript.append(['dummy KernelPoints+', totalcost])
                else:
                    transcript.append(['dummy KernelPoints-', totalcost])

                if i not in [71,72,73]:
                    # phi(P_0)
                    totalcost = totalcost + IsogenyPoint(ls[i])
                    if(s == 0):
                        transcript.append(['dummy IsogenyPoint+', totalcost])
                    else:
                        transcript.append(['dummy IsogenyPoint-', totalcost])

                    # phi(P_1)
                    totalcost = totalcost + IsogenyPoint(ls[i])
                    if(s == 0):
                        transcript.append(['dummy IsogenyPoint-', totalcost])
                    else:
                        transcript.append(['dummy IsogenyPoint+', totalcost])

                # codomain
                totalcost = totalcost + IsogenyCurve(ls[i])
                if(s == 0):
                    transcript.append(['dummy IsogenyCurve+', totalcost])
                else:
                    transcript.append(['dummy IsogenyCurve-', totalcost])

                totalcost = totalcost + IsogenyAddition(ls[i])
                if(s == 0):
                    transcript.append(['dummy IsogenyAddition+', totalcost])
                else:
                    transcript.append(['dummy IsogenyAddition-', totalcost])
                ed[i] -= 1
            k /= ls[i]

    transcript.append(['end of batch', totalcost])

def getInterm(index, sign):
    for n in range(index + 1, len(transcript)):
        if(transcript[n][0]=='end of batch'):
            return True
        else:
            if(sign):
                if(transcript[n][0] == 'real KernelPoints+'):
                    return False
            else:
                if(transcript[n][0] == 'real KernelPoints-'):
                    return False
    #print('ERRRRORRRRR. I'm a pirate.')
    return False


def getResult(argument, index, sign):
    if(argument=='Ladder+'):
        return getInterm(index, sign)     # check until end of batch if KernelPoints+
    elif(argument=='Ladder-'):
        return getInterm(index, sign)     # check until end of batch if KernelPoints-
    elif(argument=='Elligator'):
        return random.randint(0, 1)       # 50% probabilty of changing results
    elif(argument=='real KernelPoints+'):
        return False
    elif(argument=='real KernelPoints-'):
        return False
    elif(argument=='real IsogenyPoint+'):
        return getInterm(index, sign)     # check until end of batch if KernelPoints+
    elif(argument=='real IsogenyPoint-'):
        return getInterm(index, sign)     # check until end of batch if KernelPoints-
    elif(argument=='real IsogenyCurve+'):
        return False
    elif(argument=='real IsogenyCurve-'):
        return False
    elif(argument=='real IsogenyAddition+'):
        return True
    elif(argument=='real IsogenyAddition-'):
        return True
    elif(argument=='dummy KernelPoints+'):
        return getInterm(index, sign)     # check if until end of batch if KernelPoints+
    elif(argument=='dummy IsogenyPoint+'):
        return True
    elif(argument=='dummy IsogenyPoint-'):
        return True
    elif(argument=='dummy IsogenyCurve+'):
        return True
    elif(argument=='dummy IsogenyAddition+'):
        return getInterm(index, sign)     # check if until end of batch if KernelPoints+
    return True

# return whether a shared secret is valid wih a fault injection
# params
#   e: secret key
#   m: maximum number of isogenies per order
#   nsplit: number of batches
#   nfirst: number of iterations before merging batches again
def action(e, m, nsplit, nfirst, glitch=None):
    global transcript
    global totalcost
    global isog_count
    global dummy_isog
    global real_isog

    transcript = []
    totalcost = 0
    isog_count = 0
    dummy_isog = 0
    real_isog = 0

    # compute number of dummies per order
    ed = [m[i] - abs(e[i]) for i in range(n)]

    # multiple batches
    for _ in range(nfirst):
        for nb in range(nsplit):
            # compute index set for each batch
            S = [i for i in range(n) if (not(e[i] == 0 and ed[i] == 0)) and (i % nsplit) == nb]
            result = action_for_S(e, ed, S)

    # after merging the batches
    while any(e) or any(ed):
        S = [i for i in range(n) if not (e[i] == 0 and ed[i] == 0)]
        result = action_for_S(e, ed, S)

    if glitch == None:
        glitch = random.randint(0, int(totalcost)-1)
    else:
        glitch = int(glitch)

    if glitch > totalcost-1:
        return 0, totalcost


    for j in range(len(transcript)):
        if(transcript[j][1]>=glitch):
            if(getResult(transcript[j][0], j, transcript[j][0].find('+')>0) == True):
                value = 1.0    # return 1 when True
                break
            else:
                value = 0
                break
    return value, totalcost

def prob_valid(e, m, nsplit, nfirst, N, mode="1a"):
    cnt = 0
    e_saved = copy.deepcopy(e)

    # Attack 1a: hit a uniformly random location in the entire computation
    # Attack 1b: try to hit each isogeny equally often; this one didn't make it into the paper as it doesn't really provide any advantage if the number of faults is large enough
    if mode == "1a":
      for _ in range(N):
          success, _ = action(e, m, nsplit, nfirst)
          cnt += float(success) # counting the correct results
          e = copy.deepcopy(e_saved)
    elif mode == "1b":
      # get some sample totalcost
      _, totalcost = action(e, m, nsplit, nfirst)
      e = copy.deepcopy(e_saved)

      # total number of isogeny computations
      isogNum = sum(m)

      # average cost for an isogeny
      isogLength = totalcost / isogNum

      for i in range(N):
          # glitch roughly at isogeny i % isogNum
          glitchLoc = ((i % isogNum) + random.random()) * isogLength
          success, _ = action(e, m, nsplit, nfirst, glitchLoc)
          cnt += float(success)

          # restore secret key (as it was changed during the action)
          e = copy.deepcopy(e_saved)

    return cnt / N


if __name__ == "__main__":
    m = [2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
         4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
         5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
         9, 9, 9, 10, 10, 10, 10, 9, 8, 8, 8, 7, 7, 7, 7, 7, 6, 5,
         1, 2, 2]

    # SIMBA-3-8
    nsplit = 3
    nfirst = 8

    N = 1
    e = [random.randint(-m[i],m[i]) for i in range(n)]

    print(prob_valid(e, m, nsplit, nfirst, N))
    e = [1]*74
    print(prob_valid(e, m, nsplit, nfirst, N))
    e = list(m)
    print(prob_valid(e, m, nsplit, nfirst, N))
