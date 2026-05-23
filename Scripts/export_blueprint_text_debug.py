import unreal, pathlib
log_path = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/SandboxCharacter_Mover_export_debug.txt')
asset_path = '/Game/Blueprints/SandboxCharacter_Mover'
asset = unreal.EditorAssetLibrary.load_asset(asset_path)
with log_path.open('w', encoding='utf-8') as f:
    f.write(f'asset_path={asset_path}\n')
    f.write(f'asset={asset}\n')
    if not asset:
        f.write('failed to load asset\n')
    output_path = r'C:/Unreal_Projects/GASP/Scripts/SandboxCharacter_Mover_export.txt'
    task = unreal.AssetExportTask()
    task.object = asset
    task.filename = output_path
    task.automated = True
    task.prompt = False
    task.selected = False
    task.replace_identical = True
    f.write(f'task.filename={task.filename}\n')
    f.write(f'task.object={task.object}\n')
    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        f.write(f'asset_tools={asset_tools}\n')
        result = asset_tools.export_assets([task])
        f.write(f'result={result}\n')
        f.write(f'task.errors={task.errors}\n')
        f.write(f'task.exporter={task.exporter}\n')
    except Exception as e:
        f.write(f'exception={e}\n')
