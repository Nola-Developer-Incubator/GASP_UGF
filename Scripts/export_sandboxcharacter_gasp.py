import os
import unreal

asset_path = '/Game/Blueprints/SandboxCharacter_GASP'
export_dir = os.path.abspath('C:/Unreal_Projects/GASP/Scripts/ExportedBlueprint')

if not os.path.exists(export_dir):
    os.makedirs(export_dir, exist_ok=True)

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.export_assets([asset_path], export_dir)

print('export_dir=', export_dir)
for root, dirs, files in os.walk(export_dir):
    for f in files:
        print(os.path.join(root, f))
