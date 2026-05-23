from pathlib import Path
import re
p=Path(r'C:\Unreal_Projects\GASP\Scripts\ExportedBlueprint\Game\Blueprints\SandboxCharacter_GASP.T3D')
lines=p.read_text(encoding='utf-8', errors='replace').splitlines()
pattern=re.compile(r'FunctionReference=\(MemberParent=\"/Script/CoreUObject.Class\\\'/Script/Mover\\.(?:MoverComponent|CharacterMoverComponent)\\\"|VariableReference=\(MemberName=\"MoverComponent\"')
for i,l in enumerate(lines):
    if pattern.search(l):
        start=max(0,i-5)
        end=min(len(lines),i+25)
        print('===', i+1, '===')
        print('\n'.join(lines[start:end]))
        print('---')
