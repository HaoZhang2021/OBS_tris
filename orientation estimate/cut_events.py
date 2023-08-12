#cut the earthquakes waveforms according to the events catalog file 

import os
import sys
import glob
from obspy.core import read, UTCDateTime
import matplotlib.pyplot as plt
from obspy.io.xseed import Parser
from obspy.geodetics import gps2dist_azimuth
 
#stats=['TDC01','TDC02','TDC03','TDC05','TDC06','TDC07','TDC08','TDC09','TDC10','TDC12','TDC14','TDC15','TDC16','TDC18','TDC19','TDC20','TDC22','TDC23','TDC26']
stats=['TRIS']
#stats=['NIG01']
chans=['BHZ','BHE','BHN']
#chans=['HHZ','HH1','HH2','HDH']
#chans=['HHZ','HHE','HHN']
dataless = '../response/ISOLDE-1.dataless' 

for stat in stats:
  print ('Station: '+stat)
  dir = '../data-mseed/%s'%stat
  folder=os.path.exists('./%s'%stat)
  #read stlon stlat
  stfile= '../maps/stations.dat'
  op= open(stfile, "r")
  line=op.readline()
  stname =line.split(' ')[3]
  while(line!='' ): 
    stname =line.split(' ')[3]
    if (stname.split('\n')[0]==stat):  
      stlon =float(line.split(" ")[0])
      stlat =float(line.split(" ")[1])
      stdp =float(line.split(" ")[2])
      print (stlon,stlat,stname)
    line=op.readline()
  op.close()
  #read event info
  evefile= '../maps/query_tele160.csv'
  op= open(evefile, "r")
  
  outdir='./%s/tele160'%stat
  folder=os.path.exists(outdir)
  if not folder:
    os.makedirs(outdir)
  line=op.readline()
  while (line!=''):
    evetime=line.split(",")[0]
    evelon=float(line.split(",")[2])
    evelat=float(line.split(",")[1])
    evedep=float(line.split(",")[3])
    evemag=float(line.split(",")[4])
    line=op.readline()
 #   print (evetime)
    evtime=UTCDateTime(evetime.split("Z")[0])
    day=evtime.julday
    year=evtime.year
    for chan in chans:
      list_seed = glob.glob(os.path.join(dir, '*.%s.%d.%03d*.seed'%(chan,year,day)))
      list_seed.sort()
    #  print ('%s' %list_seed)
      for list in list_seed:
        try:
          trace = read(list)
         # parser = Parser(dataless)
         # paz = parser.get_paz('1D.%s..%s'%(stat,chan))
         # trace.simulate(paz_remove=paz,pre_filt=(0.001,0.01,22,25))
     
          print ('%s' %list)
         # trace.filter('bandpass', freqmin=0.07, freqmax=0.1, corners=4, zerophase=True)
          trace.plot(outfile=os.path.join('./%s/seismo_%s_%s_%s.png'%(outdir,evtime,stat,chan)) ,color='black',starttime=evtime, endtime=evtime + 5000)
          trace=trace.slice(evtime, evtime + 5000)
          trace.write('./%s/seismo_%s_%s_%s.sac'%(outdir,evtime,stat,chan), format='SAC') 
          trace=read('./%s/seismo_%s_%s_%s.sac'%(outdir,evtime,stat,chan), format='SAC')
          trace[0].stats.sac.evlo=evelon
          trace[0].stats.sac.evla=evelat
          trace[0].stats.sac.evdp=evedep*1000
          trace[0].stats.sac.stdp=stdp*1000
          trace[0].stats.sac.stla=stlat
          trace[0].stats.sac.stlo=stlon
          trace.write('./%s/seismo_%s_%s_%s.sac'%(outdir,evtime,stat,chan), format='SAC') 
        except:
          print('No data')

