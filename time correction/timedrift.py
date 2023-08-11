#caculate time drift using obspy
#2021.02

import os
import sys
import glob
import numpy as np
from obspy.core import read, UTCDateTime
import matplotlib.pyplot as plt
import obspy.signal.cross_correlation
from obspy.signal.util import smooth
import scipy

#input parameters
stats=['NIG01','TDC07']

winlen2=150 #below is parameter for caculate time drift
tshift2=50
overlap2=0.5
nstack=[1]

indir= './%s-%s/ZZ_py/'%(stats[0],stats[1]) #directory
#
def correl(trace1,trace2,winlen,overlap,tshift,delta):
  n1=0
  n2=0
  count=0
  win_samp=int(winlen*(1/delta))
  step=int(winlen*overlap*(1/delta))
  corrlen=int(2*tshift/delta+1)
  corr=np.zeros(corrlen)
  nshift=int(tshift/delta)
  npts1=len(trace1)
  npts2=len(trace2)
  while(n1<npts1-win_samp and n2<npts2-win_samp):
    cc=obspy.signal.cross_correlation.correlate(trace1[n1:n1+win_samp],trace2[n2:n2+win_samp],nshift, demean=True )
    corr= np.array(corr) + np.array(cc)
    n1+=step
    n2+=step
    count=count+1   
  corr= corr/count
  return corr
# 
def rmabnormal(data):
  var=np.var(data)
  mean=np.mean(data)
  new=[]
  count=1 
  days=[]
  print (var)
  for i in data:
    if abs(i) <3: 
       new.append(i)
       days.append(count)
    count=count+1          
  return np.array(new),np.array(days)
    
  

#read file
list_seed = glob.glob(os.path.join(indir, stats[0]+'*.asc'))
list_seed.sort()

allcorr=[]
count=0
for filepath in list_seed:
  op= open(filepath, "r")
  line=op.readline()
  lenth =int(line.split(":")[1])
  line=op.readline()
  delta =float(line.split(":")[1])
  corr=np.zeros(lenth)
  for i in range(0,lenth):
    corr[i]=(op.readline())
  op.close()
  allcorr.append(corr)
  count += 1
print ("find %d days file"%count)

#caculate 

lens=len(allcorr)

for n in nstack:
  outfile1='%s_%s_shift_stack_%d.dat'%(stats[0],stats[1],n) 
  outfile2='%s_%s_shift_stack_%d.model'%(stats[0],stats[1],n) 
  outfile3='%s_%s_shift_stack_%d.trend'%(stats[0],stats[1],n) 
 
  stallcorr=[]
  drift=[]
  amp=[]
  for i in range(0,lens-n+1):
    st=np.zeros(lenth)
    for j in range(0,n):
      st= np.array(st) +np.array(allcorr[i+j])
      stallcorr.append( np.array(st) / n) 

 
  for i in range(0,lens-n+1):
    corr2=correl(stallcorr[0],stallcorr[i],winlen2,overlap2,tshift2,delta)
    shift, value =  obspy.signal.cross_correlation.xcorr_max(corr2,abs_max=False)
    drift.append(shift)
    amp.append(value)

  drift=np.array(drift)*delta  
  drift_rm,days=rmabnormal(drift)


  slope,intercept,rvalue,pvalue,stderr=scipy.stats.linregress(days,np.array(drift_rm))
  drif=slope*lens+intercept
  ydrif=slope*365+intercept
  
  op= open(indir+outfile1, "w")
  op.write("# correlation trace number | time | amplitude\n")
  for i in range(0,lens-n+1):
    op.write('%d %lf %lf\n'%(i+1,drift[i],amp[i]))
  op.close()

  op= open(indir+outfile2, "w")
  op.write("Drift: %f s total, %f s/year, %f ppm\n"%(drif,ydrif,ydrif/31.536))
  op.close()

  op= open(indir+outfile3, "w")
  for i in range(0,lens-n+1):
    op.write('%d %lf %lf\n'%(i+1,slope*(i+1)+intercept,drift[i]-(slope*(i+1)+intercept)))
  op.close()
  print ("slope,intercept,rvalue,pvalue, stderr:")
  print( slope,intercept,rvalue,pvalue, stderr)
  plt.scatter(days,drift_rm)
  plt.plot([0,lens],[intercept,drif])
  plt.show()

