#!/bin/bash
source /cvmfs/lhaaso.ihep.ac.cn/anysw/slc5_ia64_gcc73/external/envb.sh 
cd /workfs/ybj/youzhiyong/WFCTADecode/WFCTA-zipmode
#./eventdecode.exe ES.47144.FULL.WFCTA01.es-1.20200325153549.001.dat test.root
#infile=/eos/lhaaso/raw/wfcta/2020/0330/ES.47469.FULL.WFCTA10.20200330234608.004.dat
infile=/workfs/ybj/youzhiyong/WFCTADecode/WFCTA-zipmode/Test/ES.47144.FULL.WFCTA01.es-1.20200325153549.001.dat
#./statusdecode.exe $infile ES.47144.FULL.WFCTA01.es-1.20200325153549.001.dat.status.root
./eventdecode.exe $infile ES.47144.FULL.WFCTA01.es-1.20200325153549.001.dat.event.root
