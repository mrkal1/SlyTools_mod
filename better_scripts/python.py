import mmap
import sys

files = ['1divebnt', '3ballbnt', '4slowbnt', '5dive2bnt', '6magnetbnt', '7hatbnt', '8speedbnt', '10waterbnt', '11decoybnt', '13ball2bnt', '14hackbnt', '15invisbnt', '16slomobnt', '17pitbnt', 'combocwbnt', 'combocwbnt1', 'combocwbnt2', '18freezebnt', '20raleighbnt', '21muggshotbnt', '22mzrubybnt', '19pandabnt']
source = sys.argv[1]
newname = sys.argv[2]

print (sys.argv[1])
print(sys.argv[2])

source_data = open(source, 'rb').read()
open(newname, 'wb').write(source_data)
for x in files:
  print('Doing: ' + x)
  search_data = open("E:\\mrkal\\Desktop\\Sly Work\\templates\\CluesV2\\" + x + ".find", 'rb').read()
  replace_data = open("E:\\mrkal\\Desktop\\Sly Work\\templates\\CluesV2\\rep\\" + x + ".rep", 'rb').read()
  
  fp = open(newname, 'r+')
  mm = mmap.mmap(fp.fileno(), 0)
  
  start_addr = mm.find(search_data)
  end_addr = start_addr + len(replace_data)
  try:
    mm[start_addr:end_addr] = replace_data
  except:
    print("Failed to do: " + x)
  
  mm.close()


