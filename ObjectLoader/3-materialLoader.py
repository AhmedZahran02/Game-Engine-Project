import json
import sys

if len(sys.argv) <= 1:
   print("argument missing need model name")
   exit(-1)

model_name = sys.argv[1]

def createLitMaterial (template,ambient,diffuse,specular,emission,roughness,SpecularExponent,refractionFactor,dissolveFactor,illumModel):
  newTemplate = template.copy()
  newTemplate["ambient"]=ambient
  newTemplate["diffuse"]=diffuse
  newTemplate["specular"]=specular
  newTemplate["emission"]=emission
  newTemplate["roughness"]=roughness
  newTemplate["SpecularExponent"]=SpecularExponent
  newTemplate["refractionFactor"]=refractionFactor
  newTemplate["dissolveFactor"]=dissolveFactor
  newTemplate["illumModel"]=illumModel
  newTemplate["type"]="lit"
  newTemplate["shader"]="lit"
  return newTemplate

def createLitTexturedMaterial(template,ambient,diffuse,specular,emission,roughness,SpecularExponent,refractionFactor,dissolveFactor,illumModel,textureName):
  newTemplate = template.copy()
  newTemplate = createLitMaterial(template,ambient,diffuse,specular,emission,roughness,SpecularExponent,refractionFactor,dissolveFactor,illumModel)
  newTemplate["type"]="litTextured"
  newTemplate["shader"]="lit-texture"
  newTemplate["texture"]= textureName
  newTemplate["sampler"]= "default"
  return newTemplate


with open('app.jsonc', 'r') as f:
    data = json.load(f)

# load textures
materials = data["scene"]["assets"]["materials"]
textures = data["scene"]["assets"]["textures"]

mat = {
          "pipelineState": {
            "faceCulling": {
              "enabled": False
            },
            "depthTesting": {
              "enabled": True
            }
          }
        }

data_line = open("material.lib","r+",encoding="ISO-8859-1")
data_line = data_line.readlines()

with open("material.lib_new","w+") as new_file:
  new_file.write("")
  cnt = 1
  for i in range(0,len(data_line)):
    line = data_line[i]
    words = line.split()
    if len(words) > 0 and words[0] == "newmtl":
      new_file.write("\n")
      new_file.write("newmtl " + model_name + "-mat" + str(cnt) + "\n")
      cnt+=1
    else:
      new_file.write(line)
  new_file.write("\n")


with open('material.lib_new', 'r', encoding="ISO-8859-1") as f:
    materialLoaded =False
    materialName =''
    ambient = [0,0,0]  # Ka
    diffuse = [0,0,0]  # albedo Kd
    specular = [0,0,0]# Ks
    emission = [0,0,0]# Ke
    roughness = 1
    SpecularExponent = 1 # Ns
    refractionFactor = 1
    dissolveFactor = 1
    illumModel = 0
    textureName =None
    texturePath = None
    for line in f:
        line = line.strip()
        if line.startswith("newmtl"):
            materialName = line.split()[1]
            materialLoaded =True
        if materialLoaded and line.startswith("Ns"):
            SpecularExponent = float(line.split()[1])
        if materialLoaded and line.startswith("Ka"):
            ambient = [float(line.split()[1]),float(line.split()[2]),float(line.split()[3])]
        if materialLoaded and line.startswith("Kd"):
            diffuse = [float(line.split()[1]),float(line.split()[2]),float(line.split()[3])]
        if materialLoaded and line.startswith("Ks"):
            specular = [float(line.split()[1]),float(line.split()[2]),float(line.split()[3])]
        if materialLoaded and line.startswith("Ke"):
            emission = [float(line.split()[1]),float(line.split()[2]),float(line.split()[3])]
        if materialLoaded and line.startswith("Ni"):
            refractionFactor = float(line.split()[1])
        if materialLoaded and line.startswith("d"):
            dissolveFactor = float(line.split()[1])
        if materialLoaded and line.startswith("illum"):
            illumModel = float(line.split()[1])
        if materialLoaded and line.startswith("map_Kd"):
            textureName = line.split()[1].split('.')[0]
            texturePath = line.split()[1]
        if materialLoaded and line == "":
          if textureName:
            textures[textureName] = "assets/textures/"+texturePath
            materials[materialName] = createLitTexturedMaterial(mat,ambient,diffuse,specular,emission,roughness,SpecularExponent,refractionFactor,dissolveFactor,illumModel,textureName)
          else:
            materials[materialName] = createLitMaterial(mat,ambient,diffuse,specular,emission,roughness,SpecularExponent,refractionFactor,dissolveFactor,illumModel)
          materialLoaded =False
          textureName = None



data["scene"]["assets"]["materials"] = materials
data["scene"]["assets"]["textures"] = textures          
            

with open('app.jsonc', 'w') as f:
    json.dump(data, f, indent=4)