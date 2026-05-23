import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/graph_properties2.txt')
asset = unreal.EditorAssetLibrary.load_asset('/Game/Blueprints/SandboxCharacter_Mover')
bp = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset)
with output.open('w', encoding='utf-8') as f:
    names = ['UbergraphPages', 'FunctionGraphs', 'MacroGraphs', 'InterfaceGraphs', 'SimpleConstructionScript', 'GeneratedClass', 'SkeletonGeneratedClass', 'BlueprintType', 'LastEditedDocuments']
    for name in names:
        for n in [name, name.capitalize(), name.upper(), name.lower()]:
            try:
                prop = bp.get_editor_property(n)
                f.write('bp %s: %s len=%s\n' % (n, type(prop).__name__, len(prop) if hasattr(prop, '__len__') else 'N/A'))
            except Exception as e:
                f.write('bp %s: ERROR %s\n' % (n, e))
    f.write('EventGraph nodes property?\n')
    graph = unreal.BlueprintEditorLibrary.find_graph(bp, 'EventGraph')
    for name in ['Nodes', 'nodes', 'GraphNodes', 'graph_nodes', 'NodeList', 'node_list']:
        try:
            prop = graph.get_editor_property(name)
            f.write('graph %s: %s len=%s\n' % (name, type(prop).__name__, len(prop) if hasattr(prop, '__len__') else 'N/A'))
        except Exception as e:
            f.write('graph %s: ERROR %s\n' % (name, e))
print('wrote', output)
