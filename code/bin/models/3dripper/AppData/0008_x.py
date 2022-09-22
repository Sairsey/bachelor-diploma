import Blender
print "Progress: Writing .blend file..."
import sys as sys2
sys2.stdout.flush()
Blender.Save("./BinFiles/anim_XXX.blend", True)