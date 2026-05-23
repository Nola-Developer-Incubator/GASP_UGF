import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/graph_properties.txt')
asset = unreal.EditorAssetLibrary.load_asset('/Game/Blueprints/SandboxCharacter_Mover')
bp = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset)
with output.open('w', encoding='utf-8') as f:
    names = ['UbergraphPages', 'FunctionGraphs', 'DelegateSignatureGraphs', 'Macros', 'MacroGraphs', 'InterfaceGraphs', 'SimpleConstructionScript', 'GeneratedClass', 'BlueprintType', 'SkeletonGeneratedClass', 'LastEditedDocuments', 'BlueprintCategory', 'Status', 'bIsRegeneratingOnLoad']
    for name in names:
        try:
            prop = bp.get_editor_property(name)
            f.write('%s: %s\n' % (name, type(prop).__name__))
            if hasattr(prop, '__len__'):
                f.write('  len=%s\n' % len(prop))
        except Exception as e:
            f.write('%s: ERROR %s\n' % (name, e))
    # test graph property
    graph = unreal.BlueprintEditorLibrary.find_graph(bp, 'EventGraph')
    f.write('EventGraph nodes property?\n')
    try:
        nodes = graph.get_editor_property('nodes')
        f.write('nodes type: %s len=%s\n' % (type(nodes).__name__, len(nodes)))
        for node in nodes[:20]:
            f.write('node class=%s name=%s\n' % (node.get_class().get_name(), node.get_fname()))
    except Exception as e:
        f.write('nodes error: %s\n' % e)

print('wrote', output)
