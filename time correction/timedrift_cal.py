#ambient nosie cross-correlation using obspy
#for time drift calculation

import os
import sys
import glob
import numpy as np
from obspy.core import read, UTCDateTime
import matplotlib.pyplot as plt
import obspy.signal.cross_correlation
import scipy

#input parameters
stats=['NIG01','TDC07']
chans=['HHZ','BHZ']
startday=31 #cant begin with 0
startyear=2012
endday=4
endyear=2013
winlen=500  #seconds,length of splited waveform to correlate
tshift=200  #seconds, correlation time (-value,+value)
overlap=0.5
filtype="bandpass" #filter type
bpmax=1  #if bandpass, highfrequncy	
bpmin=0.02 #if bandpass, lowfrequncy	
bpco=4  #if bandpass, corner
lpfreq=1  #if lowpass, frequncy		
lpco=2  #if lowpass, corner
hpfreq=1  #if highpass, frequncy		
hpco=2  #if highpass, corner

winlen2=150 #below is parameter for caculate time drift
tshift2=50
overlap2=0.5


outdir = './%s-%s/ZZ_py/'%(stats[0],stats[1]) #outdirectory

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
allcorr=[]

if endyear < startyear:
  print('endyear < startyear')

if endyear == startyear:
  for day in range(startday,endday+1):
    file1='../data-corrtime/%s/%s.%s.%s.%03d.seed'%(stats[0],stats[0],chans[0],startyear,day) #need to change
    print(file1)
    file2='../data-corrtime/%s/%s.%s.%s.%03d.seed'%(stats[1],stats[1],chans[1],startyear,day) #need to change
    print(file2)
    outfile='%s_%s_%d_%03d.asc'%(stats[0],stats[1],startyear,day) 
    folder=os.path.exists(outdir)
    if not folder:
      os.makedirs(outdir) 
    trace1=read(file1)[0]
    delta1=trace1.stats.delta
    trace2=read(file2)[0]
    delta2=trace2.stats.delta
    #resmaple
    if delta1 < delta2:
      trace1.resample(1/delta2)
    if delta1 > delta2:
      trace2.resample(1/delta1)
    delta=max([delta1,delta2])
    #filrter
    if filtype=="bandpass":
      trace1.filter(filtype,freqmin=bpmin,freqmax=bpmax,corners=bpco,zerophase=True)
      trace2.filter(filtype,freqmin=bpmin,freqmax=bpmax,corners=bpco,zerophase=True)
    if filtype=="lowpass":
      trace1.filter(filtype,freq=lpfeq,corners=lpco,zerophase=True)
      trace2.filter(filtype,freq=lpfeq,corners=lpco,zerophase=True)
    if filtype=="highpass":
      trace1.filter(filtype,freq=hpfeq,corners=hpco,zerophase=True)
      trace2.filter(filtype,freq=hpfeq,corners=hpco,zerophase=True)  
    corr=correl(trace1,trace2,winlen,overlap,tshift,delta) 
    allcorr.append(corr)
    corrlen=int(2*tshift/delta+1)
    op= open(outdir+outfile, "w")
    op.write("LENGTH: %d\n"%corrlen)
    op.write("DELTA: %lf\n"%delta)
    for i in corr:
      op.write('%lf\n'%i)
    op.close()

if endyear > startyear:
  for year in range(startyear, endyear+1):
    if year==startyear:
      startday2=startday
    else: 
      startday2=1

    if year==endyear:
      endday2=endday
    elif year%100==0 and year%400==0: 
      endday2=366
    elif year%100!=0 and year%4==0:
      endday2=366
    else:
      enday2=365
 
    for day in range(startday2,endday2+1):
      file1='../data-mseed/%s/%s.%s.%s.%03d.mseed'%(stats[0],stats[0],chans[0],year,day) #need to change
      print(file1)
      file2='../data-mseed/%s/%s.%s.%s.%03d.seed'%(stats[1],stats[1],chans[1],year,day) #need to change
      print(file2)
      outfile='%s_%s_%d_%03d.asc'%(stats[0],stats[1],year,day)
      folder=os.path.exists(outdir)
      if not folder:
        os.makedirs(outdir) 
      trace1=read(file1)[0]
      delta1=trace1.stats.delta
      trace2=read(file2)[0]
      delta2=trace2.stats.delta
      #resample
      if delta1 < delta2:
        trace1.resample(1/delta2)
      if delta1 > delta2:
        trace2.resample(1/delta1)
      delta=max([delta1,delta2])
      #filrter
      if filtype=="bandpass":
        trace1.filter(filtype,freqmin=bpmin,freqmax=bpmax,corners=bpco,zerophase=True)
        trace2.filter(filtype,freqmin=bpmin,freqmax=bpmax,corners=bpco,zerophase=True)
      if filtype=="lowpass":
        trace1.filter(filtype,freq=lpfeq,corners=lpco,zerophase=True)
        trace2.filter(filtype,freq=lpfeq,corners=lpco,zerophase=True)
      if filtype=="highpass":
        trace1.filter(filtype,freq=hpfeq,corners=hpco,zerophase=True)
        trace2.filter(filtype,freq=hpfeq,corners=hpco,zerophase=True) 
      
      corr=correl(trace1,trace2,winlen,overlap,tshift,delta)
      allcorr.append(corr)
      corrlen=int(2*tshift/delta+1)
      op= open(outdir+outfile, "w")
      op.write("LENGTH: %d\n"%corrlen)
      op.write("DELTA: %lf\n"%delta)
      for i in corr:
        op.write('%lf\n'%i)
      op.close()

print ("finish correlation caculation ")
#run the scripts for gmt plot 

os.system('./stack_corr.csh %s %s %s 1 1'%(outdir,stats[0],stats[1]))
os.system('./stack_corr.csh %s %s %s 10 1'%(outdir,stats[0],stats[1]))
#os.system('./time_stack.csh %s %s %s 1 1'%(outdir,stats[0],stats[1]))
#os.system('./time_corr.csh %s %s %s 1 '%(outdir,stats[0],stats[1]))


#caculate the time drift



nstack=[1]

lens=len(allcorr)

for n in nstack:
  outfile1='%s_%s_shift_stack_%d.dat'%(stats[0],stats[1],n) 
  outfile2='%s_%s_shift_stack_%d.model'%(stats[0],stats[1],n) 
  outfile3='%s_%s_shift_stack_%d.trend'%(stats[0],stats[1],n) 
 
  stallcorr=[]
  drift=[]
  amp=[]
  for i in range(0,lens-n+1):
    st=np.zeros(corrlen)
    for j in range(0,n):
      st= np.array(st) +np.array(allcorr[i+j])
      stallcorr.append( np.array(st) / n) 

 
  for i in range(0,lens-n+1):
    corr2=correl(stallcorr[0],stallcorr[i],winlen2,overlap2,tshift2,delta)
    shift, value =  obspy.signal.cross_correlation.xcorr_max(corr2,abs_max=False)
    drift.append(shift)
    amp.append(value)

  drift=np.array(drift)*delta
  slope,intercept,rvalue,pvalue,stderr=scipy.stats.linregress(range(1,lens-n+2),np.array(drift))
  drif=slope*lens+intercept
  ydrif=slope*365+intercept
  
  op= open(outdir+outfile1, "w")
  op.write("# correlation trace number | time | amplitude\n")
  for i in range(0,lens-n+1):
    op.write('%d %lf %lf\n'%(i+1,drift[i],amp[i]))
  op.close()

  op= open(outdir+outfile2, "w")
  op.write("Drift: %f s total, %f s/year, %f ppm\n"%(drif,ydrif,ydrif/31.536))
  op.close()

  op= open(outdir+outfile3, "w")
  for i in range(0,lens-n+1):
    op.write('%d %lf %lf\n'%(i+1,slope*(i+1)+intercept,drift[i]-(slope*(i+1)+intercept)))
  op.close()
  print ("slope,intercept,rvalue,pvalue, stderr:")
  print (slope,intercept,rvalue,pvalue, stderr)
  plt.scatter(range(1,lens-n+2),np.array(drift))
  plt.show()
os.system('cp ./ZZ_all.gmt %s '%(outdir))
