
BLENDER="C:\Program Files\Blender Foundation\Blender\blender.exe"

all : sea.pnct sea.scene

sea.pnct : submarine.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- submarine.blend sea.pnct
sea.scene : submarine.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- submarine.blend sea.scene
