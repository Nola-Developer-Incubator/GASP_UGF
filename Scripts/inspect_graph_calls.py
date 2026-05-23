import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/graph_calls.txt')
asset = unreal.EditorAssetLibrary.load_asset('/Game/Blueprints/SandboxCharacter_Mover')
bp = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset)
graph = unreal.BlueprintEditorLibrary.find_graph(bp, 'EventGraph')

test_names = [
    'GetNodes', 'GetAllNodes', 'GetGraphNodes', 'GetNodesArray', 'GetNodeArray',
    'GetEditableNodes', 'GetNodesInternal', 'GetNodesList', 'GetAllNodesOfClass',
    'GetNodeCount', 'GetGraph', 'GetAllSubgraphs', 'GetGraphNodes',
]
with output.open('w', encoding='utf-8') as f:
    for name in test_names:
        try:
            result = unreal.BlueprintEditorLibrary.call_method(graph, name)
            f.write('%s: %s\n' % (name, type(result).__name__))
            if hasattr(result, '__len__'):
                f.write('  len=%s\n' % len(result))
        except Exception as e:
            f.write('%s: ERROR %s\n' % (name, e))
    f.write('graph class methods via custom call 1\n')
    for name in ['Nodes', 'GetNodes', 'GetAllNodes']:
        try:
            result = unreal.BlueprintEditorLibrary.call_method(graph, name)
            f.write('call %s: %s\n' % (name, type(result).__name__))
        except Exception as e:
            f.write('call %s: ERROR %s\n' % (name, e))
print('wrote', output)
