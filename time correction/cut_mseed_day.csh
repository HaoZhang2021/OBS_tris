#!/bin/csh
#cut whole mseed file into daily files

set x=19
while ($x<20)

mkdir TDC$x 	
mseedcut --file-length=DAY --output-dir=./TDC$x   TDC$x*corr.seed
mseedrename --output-dir=./TDC$x --transfer-mode=MOVE --include-pattern='tdc*' --template=%S.%C.%Y.%j.seed ./TDC$x
@ x ++
end
