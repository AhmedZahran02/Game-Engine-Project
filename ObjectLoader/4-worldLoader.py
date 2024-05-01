import json
import os

files = os.listdir("./output/")
meshes = [os.path.splitext(file)[0] for file in files]

with open('app.jsonc', 'r') as f:
    data = json.load(f)

world = data["scene"]["world"]
entity =       {
        "position": [0, 0, 0],
        "rotation": [0, 0, 0],
        "children": []}



for meshName in meshes:
    child = {
            "components": [
              {
                "type": "Mesh Renderer",
                "mesh": "",
                "material": ""
              }
            ]
          }
    child["components"][0]["mesh"]=meshName
    child["components"][0]["material"]=meshName.split('-')[-2] +"-"+ meshName.split('-')[-1]
    entity["children"].append(child)

world.append(entity)         
            
with open('app.jsonc', 'w') as f:
    json.dump(data, f, indent=4)