from pathlib import Path
p=Path(r'C:\Unreal_Projects\GASP\Scripts\ExportedBlueprint\Game\Blueprints\SandboxCharacter_GASP.T3D')
lines=p.read_text(encoding='utf-8', errors='replace').splitlines()
for i,line in enumerate(lines):
    if 'Mover.MoverComponent' in line or 'Mover.CharacterMoverComponent' in line:
        start=max(0,i-5)
        end=min(len(lines),i+25)
        print('===', i+1, '===')
        print('\n'.join(lines[start:end]))
        print('---')
