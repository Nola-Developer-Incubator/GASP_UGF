import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/editor_asset_library.txt')
asset = unreal.EditorAssetLibrary.load_asset('/Game/Blueprints/SandboxCharacter_Mover')
bp = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset)
graph = unreal.BlueprintEditorLibrary.find_graph(bp, 'EventGraph')
with output.open('w', encoding='utf-8') as f:
    for name in ['Nodes', 'nodes', 'UbergraphPages', 'FunctionGraphs']:
        try:
            prop = unreal.EditorAssetLibrary.get_editor_property(graph, name)
            f.write('graph %s: %s len=%s\n' % (name, type(prop).__name__, len(prop) if hasattr(prop, '__len__') else 'N/A'))
        except Exception as e:
            f.write('graph %s: ERROR %s\n' % (name, e))
    for name in ['UbergraphPages', 'FunctionGraphs']:
        try:
            prop = unreal.EditorAssetLibrary.get_editor_property(bp, name)
            f.write('bp %s: %s len=%s\n' % (name, type(prop).__name__, len(prop) if hasattr(prop, '__len__') else 'N/A'))
        except Exception as e:
            f.write('bp %s: ERROR %s\n' % (name, e))
print('wrote', output)
