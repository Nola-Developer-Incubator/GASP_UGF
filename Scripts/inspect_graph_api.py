import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/graph_api.txt')
asset = unreal.EditorAssetLibrary.load_asset('/Game/Blueprints/SandboxCharacter_Mover')
with output.open('w', encoding='utf-8') as f:
    f.write('asset class: %s\n' % asset.get_class().get_name())
    bp = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset)
    f.write('blueprint class: %s\n' % bp.get_class().get_name())
    graphs = unreal.BlueprintEditorLibrary.find_graph(bp, 'EventGraph')
    f.write('EventGraph type: %s\n' % (type(graphs).__name__))
    for name in sorted(dir(graphs)):
        if not name.startswith('_'):
            f.write(name + '\n')
    f.write('\nGraph nodes count?\n')
    try:
        nodes = graphs.get_nodes()
        f.write('get_nodes available: %s\n' % len(nodes))
        for node in nodes[:20]:
            f.write('node class: %s\n' % node.get_class().get_name())
            f.write('node name: %s\n' % node.get_fname())
    except Exception as e:
        f.write('get_nodes error: %s\n' % e)
    f.write('\nGraph pins?\n')
    try:
        pins = graphs.get_pins()
        f.write('get_pins available: %s\n' % len(pins))
    except Exception as e:
        f.write('get_pins error: %s\n' % e)

print('wrote', output)
