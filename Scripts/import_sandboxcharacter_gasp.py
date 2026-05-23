import unreal
import os

# Corrected T3D export path
source_t3d = r"C:\Unreal_Projects\GASP\Scripts\ExportedBlueprint\Game\Blueprints\SandboxCharacter_GASP.T3D"

destination_path = "/Game/Blueprints"
destination_name = "SandboxCharacter_GASP"

if not os.path.exists(source_t3d):
    raise FileNotFoundError(f"T3D source file not found: {source_t3d}")

print(f"Importing T3D from: {source_t3d}")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
import_task = unreal.AssetImportTask()
import_task.set_editor_property('filename', source_t3d)
import_task.set_editor_property('destination_path', destination_path)
import_task.set_editor_property('destination_name', destination_name)
import_task.set_editor_property('replace_existing', True)
import_task.set_editor_property('automated', True)
import_task.set_editor_property('save', True)

asset_tools.import_asset_tasks([import_task])

imported = import_task.get_editor_property('imported_object_paths')
print('imported_object_paths=', imported)

if not imported or len(imported) == 0:
    raise RuntimeError('No assets were imported. Check the T3D file and import settings.')

print('Import complete.')
