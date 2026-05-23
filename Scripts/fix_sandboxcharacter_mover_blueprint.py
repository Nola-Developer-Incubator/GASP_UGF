import os
import re

import unreal

BP_ASSET_PATH = "/Game/Blueprints/SandboxCharacter_Mover"
EXPORT_DIRNAME = "Scripts/ExportedBlueprint"
T3D_SUBPATH = os.path.join("Game", "Blueprints", "SandboxCharacter_Mover.T3D")

MOVER_COMPONENT_LINK = "K2Node_VariableGet_23 042556C4458163EA492B9E806230F627"
MOVER_COMPONENT_CLASS = "/Script/CoreUObject.Class'/Script/Mover.MoverComponent'"
CHARACTER_MOVER_COMPONENT_CLASS = "/Script/CoreUObject.Class'/Script/Mover.CharacterMoverComponent'"
MOVER_COMPONENT_VARIABLE_PIN_ID = "042556C4458163EA492B9E806230F627"


def get_export_path(base_dir: str) -> str:
    return os.path.join(base_dir, T3D_SUBPATH)


def export_blueprint(asset_path: str, export_dir: str) -> str:
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    full_export_dir = os.path.abspath(export_dir)
    if not os.path.exists(full_export_dir):
        os.makedirs(full_export_dir, exist_ok=True)

    unreal.log("Exporting blueprint %s to %s" % (asset_path, full_export_dir))
    asset_tools.export_assets([asset_path], full_export_dir)

    t3d_path = get_export_path(full_export_dir)
    if not os.path.exists(t3d_path):
        raise FileNotFoundError("Exported T3D not found at %s" % t3d_path)

    unreal.log("Exported T3D path: %s" % t3d_path)
    return t3d_path


def find_broken_target_pins(lines, class_name):
    broken = []
    for idx, line in enumerate(lines):
        if 'PinName="self"' in line and class_name in line and 'LinkedTo=' not in line:
            broken.append((idx, line.strip()))
    return broken


def patch_target_self_pins(lines, target_class, link_target):
    patched = 0
    for idx, line in enumerate(lines):
        if 'PinName="self"' in line and target_class in line and 'LinkedTo=' not in line:
            if 'PinType.bSerializeAsSinglePrecisionFloat=False,' in line:
                lines[idx] = line.replace(
                    'PinType.bSerializeAsSinglePrecisionFloat=False,',
                    'PinType.bSerializeAsSinglePrecisionFloat=False,LinkedTo=(%s,),'
                    % link_target,
                )
                patched += 1
    return patched


def patch_mover_component_variable_type(lines):
    patched = 0
    for idx, line in enumerate(lines):
        if (
            MOVER_COMPONENT_VARIABLE_PIN_ID in line
            and 'PinName="MoverComponent"' in line
            and MOVER_COMPONENT_CLASS in line
        ):
            lines[idx] = line.replace(MOVER_COMPONENT_CLASS, CHARACTER_MOVER_COMPONENT_CLASS)
            patched += 1
    return patched


def import_t3d_asset(t3d_path: str, destination_path: str, destination_name: str) -> list:
    unreal.log("Importing patched T3D asset from %s" % t3d_path)
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', t3d_path)
    task.set_editor_property('destination_path', destination_path)
    task.set_editor_property('destination_name', destination_name)
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('automated', True)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    imported_paths = task.get_editor_property('imported_object_paths') or []
    unreal.log('Imported asset paths: %s' % imported_paths)
    return imported_paths


def patch_t3d_file(t3d_path: str) -> int:
    with open(t3d_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    broken_mover = find_broken_target_pins(lines, MOVER_COMPONENT_CLASS)
    broken_character = find_broken_target_pins(lines, CHARACTER_MOVER_COMPONENT_CLASS)

    unreal.log("Found %d unlinked MoverComponent target pins." % len(broken_mover))
    unreal.log("Found %d unlinked CharacterMoverComponent target pins." % len(broken_character))

    patched_mover = patch_target_self_pins(lines, MOVER_COMPONENT_CLASS, MOVER_COMPONENT_LINK)
    patched_character = patch_target_self_pins(lines, CHARACTER_MOVER_COMPONENT_CLASS, MOVER_COMPONENT_LINK)
    patched_variable_type = patch_mover_component_variable_type(lines)
    patched = patched_mover + patched_character + patched_variable_type

    if patched > 0:
        with open(t3d_path, 'w', encoding='utf-8', errors='ignore') as f:
            f.writelines(lines)
        unreal.log("Patched %d target pin(s) in %s" % (patched, t3d_path))
    else:
        unreal.log("No self pins required patching.")

    return patched


def main():
    workspace_root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
    export_dir = os.path.join(workspace_root, EXPORT_DIRNAME)
    t3d_path = export_blueprint(BP_ASSET_PATH, export_dir)

    patched = patch_t3d_file(t3d_path)
    if patched == 0:
        unreal.log_warning("No patch changes were applied. The blueprint may already be fixed or the target pin pattern is unexpected.")
        return

    imported_paths = import_t3d_asset(t3d_path, "/Game/Blueprints", "SandboxCharacter_Mover")
    if imported_paths:
        unreal.log("Patch complete and asset re-imported successfully.")
        unreal.log("Imported paths: %s" % imported_paths)
    else:
        unreal.log_warning("Patch applied but re-import did not return any imported paths. Verify manually in the Content Browser.")

    unreal.log("T3D file updated: %s" % t3d_path)


if __name__ == '__main__':
    main()
