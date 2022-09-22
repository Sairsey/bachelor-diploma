from Blender import *
import bpy
import BPyMessages
 
sce = bpy.data.scenes.active
for object in sce.objects:
    if object.sel == 1:
            object.sel = 0
             
             
def objectSelect(type, scene):
    objCount = 0
    for object in scene.objects:
        if object.type == type:
            object.sel = 1
            objCount += 1
     
objectSelect('Armature', sce)
objectSelect('Mesh', sce)