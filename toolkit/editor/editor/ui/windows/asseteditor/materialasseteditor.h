#pragma once
#include "asseteditor.h"
namespace Presentation
{
void MaterialEditor(AssetEditor* assetEditor, AssetEditorItem* item);
void MaterialSetup(AssetEditorItem* item);
void MaterialSave(AssetEditor* assetEditor, AssetEditorItem* item);
void MaterialDiscard(AssetEditor* assetEditor, AssetEditorItem* item);
}