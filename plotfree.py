from matplotlib import pyplot as plt
from scipy import optimize
import numpy as np
from scipy import interpolate

phases=['6','121','3','7','633','15','12','233','1','0','1612','336','315','314','1233','13','17'] #The first one is the base
# phases=['6','121','3','7','633','15','12','233','1','0','1612','336','315','314','1233','13','17'] #The first one is the base
# phases=['3','121','233','7','633','15','12','6','1','0','1612','336','315','314','1233','13','17','126'] #The first one is the base
# phases=['233','12','3','17','7','0','126']

phasename={'126':'Z-A','1612':'PL','633':'FCC','7':'Gyroid','12':'L','3':'C','6':'Sigma','17':'G-A','233':'BCC','0':'Random','15':'A15','121':'Laves','1':'Gyroid','314':'C14','315':'C15','336':'C36','1233':'BCC-A','13':'C-A','17':'G-A'}

def f_1(x, A, B):
    return A*x + B

def f_2(x, A, B, C):
    return A*x*x + B*x + C

def f_3(x, A, B, C, D):
    return A*x*x*x + B*x*x + C*x + D

def f_4(x, A, B, C, D, E):
    return A*x*x*x*x + B*x*x*x + C*x*x + D*x + E

def f_5(x, A, B, C, D, E, F):
    return A*x*x*x*x*x + B*x*x*x*x + C*x*x*x + D*x*x + E*x + F

def f_10(x, A, B, C, D, E, F, G, H, I, J, K):
    return A*x*x*x*x*x*x*x*x*x*x + B*x*x*x*x*x*x*x*x*x + C*x*x*x*x*x*x*x*x + D*x*x*x*x*x*x*x + E*x*x*x*x*x*x + F*x*x*x*x*x + G*x*x*x*x + H*x*x*x + I*x*x + J*x + K

for i, phase in enumerate(phases): 
    try:
        
        # if phase=='3':
        #     fromstring='a0.120b0.560'
        #     endstring='a0.440b0.240'
        # if phase=='1233':
        #     fromstring='a0.200b0.480'
        #     endstring='a0.400b0.280'
        # # # elif phase=='0':
        # # #     fromstring='a0.010b0.230'
        # # #     endstring='a0.030b0.210'
        # else:
        fromstring='a0.020b0.280'
        endstring='a0.280b0.020'
        
        with open('freeE/'+phase+'/'+fromstring+'-'+endstring) as free:
            result = free.readlines()
        free={}
        for item in result:
            free[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
        if i == 0:
            origin=free.copy()
        for key, value in free.items():
            # pass
            free[key] = str(float(free[key]) - float(origin[key]))
        freeplotx=[float(item) for item in free.keys()]
        freeploty=[float(item) for item in free.values()]
        # plt.scatter(freeplotx,freeploty)
        # plt.plot(freeplotx,freeploty)

        # A, B = optimize.curve_fit(f_1, freeplotx[0::], freeploty[0::])[0]
        A, B, C = optimize.curve_fit(f_2, freeplotx[0::], freeploty[0::])[0]
        # A, B, C, D = optimize.curve_fit(f_3, freeplotx[0::], freeploty[0::])[0]
        # A, B, C, D, E= optimize.curve_fit(f_4, freeplotx[0::], freeploty[0::])[0]
        # A, B, C, D, E, F= optimize.curve_fit(f_5, freeplotx[0::], freeploty[0::])[0]
        # A, B, C, D, E, F, G, H, I, J, K= optimize.curve_fit(f_10, freeplotx[0::], freeploty[0::])[0]
        
        # x = np.linspace(freeplotx[0]-0.02, freeplotx[-1]+0.02, 1000)
        # x = np.linspace(min(freeplotx)-0.03, max(freeplotx)+0.03, 100)
        # y=f_1(x,A,B)
        # y=f_2(x,A,B,C)
        # y=f_3(x,A,B,C,D)
        # y=f_4(x,A,B,C,D,E)
        # y=f_5(x,A,B,C,D,E,F)
        # y=f_10(x,A,B,C,D,E,F,G,H,I,J,K)
        # print(phasename[phase], '0.760', f_4(0.02,A,B,C,D,E))

        func = interpolate.interp1d(freeplotx,freeploty,kind='quadratic')
        x = np.linspace(min(freeplotx), max(freeplotx), 100)
        y = func(x)

        plt.plot(x,y,label=phasename[phase],linewidth='1')

    except Exception as identifier:
        print(phase,identifier)

plt.legend()
# plt.title(fromstring+'-'+endstring)
plt.xlabel('$f_A$')
plt.ylabel('$(F-F_{\sigma})/k_BT$')
plt.show()