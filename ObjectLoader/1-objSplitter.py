import os
import sys
output_dir = "./output/"

if len(sys.argv) <= 1:
   print("argument missing need model name")
   exit(-1)

model_name = sys.argv[1]

data_line = open("model.obj","r+", encoding="ISO-8859-1")
data_line = data_line.readlines()
lines = 1
with open("model_new.obj","w+", encoding="ISO-8859-1") as new_file:
    new_file.write("")
    cnt1 = 1
    cnt2 = 1
    for i in range(0,len(data_line)):
        line = data_line[i]
        words = line.split()
        if len(words) > 0 and words[0] == "usemtl":
            model_name = sys.argv[1]
            new_file.write("usemtl " + model_name + "-mat" + str(cnt1) + "\n")
            cnt1+=1
        elif line.startswith("o ") or line.startswith("g "):
            line = "o " + model_name + "-" + str(cnt2)
            cnt2+=1
            new_file.write(line + "\n")
        elif line.startswith("#"):
            pass
        else:
            new_file.write(line)
    new_file.write("\n")
    lines = new_file.readlines()
    

with open('model_new.obj', 'r' , encoding="ISO-8859-1") as f:
    lines = f.readlines()

o_indices = [i for i, line in enumerate(lines) if line.startswith("o ") or line.startswith("g ")]

objectNames = [line.split()[1] for i, line in enumerate(lines) if line.startswith("o ") or line.startswith("g ")]

separated_objects = []
for i in range(len(o_indices)):
    if i == len(o_indices) - 1:
        separated_objects.append(lines[o_indices[i]:])
    else:
        separated_objects.append(lines[o_indices[i]:o_indices[i+1]])

common_parts = []
for objNum,object in enumerate(separated_objects):
    usemtl_indices = [i for i, line in enumerate(object) if line.startswith("usemtl")]
    materialNames = [line.split()[1] for i, line in enumerate(object) if line.startswith("usemtl")]
    common_part = ''.join(object[:usemtl_indices[0]])
    
    common_parts.append(common_part)
    
    separated_parts = []
    for i in range(len(usemtl_indices)):
        if i == len(usemtl_indices) - 1:
            separated_parts.append(''.join(object[usemtl_indices[i]+1:]))
        else:
            separated_parts.append(''.join(object[usemtl_indices[i]+1:usemtl_indices[i+1]]))

    finalCommonPart = ''.join(common_parts)
    for idx, part in enumerate(separated_parts):
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        with open(f'{output_dir}{objectNames[objNum]}-{materialNames[idx]}.obj', 'w') as part_file:
            part_file.write(finalCommonPart + part)