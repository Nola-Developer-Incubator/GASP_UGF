from pathlib import Path
path = Path('Scripts/ExportedBlueprint/Game/Blueprints/SandboxCharacter_GASP.T3D')
text = path.read_text(encoding='utf-8', errors='replace').splitlines()
in_graph = False
nest = 0
graph = []
target = 'Begin Object Class=/Script/Engine.EdGraph Name="ProduceInput_MERGED"'
for line in text:
    if target in line:
        in_graph = True
        nest = 1
        graph = [line]
        continue
    if in_graph:
        graph.append(line)
        if line.strip().startswith('Begin Object '):
            nest += 1
        elif line.strip() == 'End Object':
            nest -= 1
        if nest == 0:
            break
if not in_graph:
    print('NO GRAPH')
    raise SystemExit(1)
for i, line in enumerate(graph):
    if 'ErrorType=1' in line:
        j = i
        while j >= 0 and not graph[j].strip().startswith('Begin Object Class=/Script/BlueprintGraph.'):
            j -= 1
        if j >= 0:
            print('ERROR NODE header:', graph[j].strip())
            print(' at graph line', i)
        else:
            print('ERROR without header near graph line', i)
