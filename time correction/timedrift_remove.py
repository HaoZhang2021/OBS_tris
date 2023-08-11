#correct linear skew (time drift) of mseed using qedit 

import os
import sys
import glob
#from obspy.core import read, UTCDateTime
import matplotlib.pyplot as plt
dir='../data-mseed'
station='TDC18'
list_seed = glob.glob(os.path.join(dir, station+'*'+'*.seed'))
list_seed.sort()
initial_time_interval='0.0,0:0:0.0'
end_time_interval='0.0,0:0:0.-4242'
starttime='2012.26,21:39:39.0000'
endtime='2012.333,22:14:05.9800'
for filename in list_seed:
  (filepath,name) = os.path.split(filename)
  (filein,extension) = os.path.splitext(name)
  fileout='%s_corr.seed'%filein
  print fileout
  os.system("qedit %s <<EOF\n from 2012.183,00:00:00.0000 to 2013.20,00:00:00.0000 add_trend corr 0.0,0:0:-1.0 0.0,0:0:-1.0\n from %s to %s add_trend corr %s %s\n write %s \n quit\n"%(filename,starttime,endtime,initial_time_interval,end_time_interval,fileout))

