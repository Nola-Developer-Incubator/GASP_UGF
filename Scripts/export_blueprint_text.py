import unreal

asset_path = '/Game/Blueprints/SandboxCharacter_Mover'
asset = unreal.EditorAssetLibrary.load_asset(asset_path)
if not asset:
    raise RuntimeError(f'Failed to load asset {asset_path}')

output_path = r'C:/Unreal_Projects/GASP/Scripts/SandboxCharacter_Mover_export.txt'

task = unreal.AssetExportTask()
task.object = asset
task.filename = output_path
task.automated = True
task.prompt = False
task.selected = False
task.replace_identical = True
# no exporter set, let the engine choose

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
result = asset_tools.export_assets([task])
print('result', result)
print('errors', task.errors)
print('filename', task.filename)
print('exporter', task.exporter)
