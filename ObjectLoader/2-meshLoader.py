import json
import os

files = os.listdir("./output/")
meshes = [os.path.splitext(file)[0] for file in files]

with open('app.jsonc', 'r') as f:
    data = json.load(f)

mesh = data["scene"]["assets"]["meshes"]

for meshName in meshes:
    mesh[meshName]="assets/models/"+meshName+".obj"

data["scene"]["assets"]["meshes"] = mesh         
            

with open('app.jsonc', 'w') as f:
    json.dump(data, f, indent=4)