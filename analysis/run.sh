BP_SIZE=$1
PD=$2
FW=$3

#BP="BP${BP_SIZE}_${PD}_${FW}_opt_path"
#BP="BP${BP_SIZE}_${PD}_${FW}_path"
 BP="BP${BP_SIZE}_${PD}_${FW}_LP_path"
#BP="BP${BP_SIZE}_${PD}_${FW}"

  echo ***perlbench400 odbp $BP 0.00 0.00 TR
 ./npTO1p.py perlbench400 odbp $BP 0.00 0.00 TR
  echo ***bzip401 odbp $BP 0.00 0.00 TR
 ./npTO1p.py bzip401 odbp $BP 0.00 0.00 TR
#echo ***./npTO1p.py gcc403 odbp $BP 0.00 0.00 TR
#/npTO1p.py gcc403 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py mcf429 odbp $BP 0.00 0.00 TR
 ./npTO1p.py mcf429 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py hmmer456 odbp $BP 0.00 0.00 TR
 ./npTO1p.py hmmer456 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py sjeng458 odbp $BP 0.00 0.00 TR
 ./npTO1p.py sjeng458 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py libquantum462 odbp $BP 0.00 0.00 TR
 ./npTO1p.py libquantum462 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py h264ref464 odbp $BP 0.00 0.00 TR
 ./npTO1p.py h264ref464 odbp $BP 0.00 0.00 TR
 # echo ***./npTO1p.py gobmk445 odbp $BP 0.00 0.00 TR
 #./npTO1p.py gobmk445 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py omnetpp471 odbp $BP 0.00 0.00 TR
 ./npTO1p.py omnetpp471 odbp $BP 0.00 0.00 TR
  echo ***./npTO1p.py astar473 odbp $BP 0.00 0.00 TR
 ./npTO1p.py astar473 odbp $BP 0.00 0.00 TR
 # echo ***./npTO1p.py xalanc483 odbp $BP 0.00 0.00 TR
 #./npTO1p.py xalanc483 odbp $BP 0.00 0.00 TR

# echo ***perlbench400 nodbp $BP
#./npTO1p.py perlbench400 nodbp $BP
# echo ***bzip401 nodbp $BP
#./npTO1p.py bzip401 nodbp $BP
# echo ***./npTO1p.py gcc403 nodbp $BP
#./npTO1p.py gcc403 nodbp $BP
# echo ***./npTO1p.py mcf429 nodbp $BP
#./npTO1p.py mcf429 nodbp $BP
# echo ***./npTO1p.py hmmer456 nodbp $BP
#./npTO1p.py hmmer456 nodbp $BP
# echo ***./npTO1p.py sjeng458 nodbp $BP
#./npTO1p.py sjeng458 nodbp $BP
# echo ***./npTO1p.py libquantum462 nodbp $BP
#./npTO1p.py libquantum462 nodbp $BP
# echo ***./npTO1p.py h264ref464 nodbp $BP
#./npTO1p.py h264ref464 nodbp $BP
# #echo ***./npTO1p.py gobmk445 nodbp $BP
# #./npTO1p.py gobmk445 nodbp $BP
# echo ***./npTO1p.py omnetpp471 nodbp $BP
#./npTO1p.py omnetpp471 nodbp $BP
# echo ***./npTO1p.py astar473 nodbp $BP
#./npTO1p.py astar473 nodbp $BP
##echo ***./npTO1p.py xalanc483 nodbp $BP
##./npTO1p.py xalanc483 nodbp $BP
