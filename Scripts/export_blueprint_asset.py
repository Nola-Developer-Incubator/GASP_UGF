import unreal, pathlib

asset_path = '/Game/Blueprints/SandboxCharacter_Mover'
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
export_dir = r'C:/Unreal_Projects/GASP/Scripts/ExportedBlueprint'
pathlib.Path(export_dir).mkdir(parents=True, exist_ok=True)
asset_tools.export_assets([asset_path], export_dir)
print('exported', asset_path, 'to', export_dir)
