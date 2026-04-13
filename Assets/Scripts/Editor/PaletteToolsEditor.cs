using UnityEditor;
using UnityEngine;

public class PaletteToolsEditor : EditorWindow
{
    private ColorPalette palette;

    [MenuItem("Tools/Palette/Apply Palette To Scene")]
    public static void ShowWindow()
    {
        GetWindow<PaletteToolsEditor>("Palette Apply");
    }

    private void OnGUI()
    {
        GUILayout.Label("Apply palette colors to all binders in the open scenes", EditorStyles.boldLabel);

        palette = (ColorPalette)EditorGUILayout.ObjectField("Palette", palette, typeof(ColorPalette), false);

        GUI.enabled = palette != null;

        if (GUILayout.Button("Apply Palette"))
        {
            ApplyPaletteToScene();
        }

        GUI.enabled = true;
    }

    private void ApplyPaletteToScene()
    {
        PaletteColorBinder[] binders = FindObjectsByType<PaletteColorBinder>(
            FindObjectsInactive.Include,
            FindObjectsSortMode.None
        );

        int count = 0;

        foreach (var binder in binders)
        {
            Undo.RecordObject(binder, "Apply Palette");
            binder.palette = palette;

            if (binder.spriteRendererTarget != null)
                Undo.RecordObject(binder.spriteRendererTarget, "Apply Palette");

            if (binder.imageTarget != null)
                Undo.RecordObject(binder.imageTarget, "Apply Palette");

            if (binder.tmpTextTarget != null)
                Undo.RecordObject(binder.tmpTextTarget, "Apply Palette");

            if (binder.rendererTarget != null && binder.rendererTarget.sharedMaterial != null)
                Undo.RecordObject(binder.rendererTarget.sharedMaterial, "Apply Palette");

            binder.ApplyColor();

            EditorUtility.SetDirty(binder);

            if (binder.spriteRendererTarget != null)
                EditorUtility.SetDirty(binder.spriteRendererTarget);

            if (binder.imageTarget != null)
                EditorUtility.SetDirty(binder.imageTarget);

            if (binder.tmpTextTarget != null)
                EditorUtility.SetDirty(binder.tmpTextTarget);

            if (binder.rendererTarget != null && binder.rendererTarget.sharedMaterial != null)
                EditorUtility.SetDirty(binder.rendererTarget.sharedMaterial);

            count++;
        }

        Debug.Log($"Palette applied to {count} objects.");
    }
}