using UnityEngine;
using UnityEditor;
using UnityEngine.UI;
using Aetherwave;

public class SceneCleanup
{
    [MenuItem("Aetherwave/Clean Scene - Remove Conflicts")]
    public static void CleanScene()
    {
        Debug.Log("🧹 Starting scene cleanup to remove script conflicts...");
        
        // Find all GameObjects with conflicting MonoBehaviour scripts
        var minimalGalleries = Object.FindObjectsByType<MinimalGallery>(FindObjectsSortMode.None);
        var apiTestGalleries = Object.FindObjectsByType<APITestGallery>(FindObjectsSortMode.None);
        var oldImageManagers = Object.FindObjectsByType<ImageDisplayManager>(FindObjectsSortMode.None);
        
        int removed = 0;
        
        // Remove conflicting scripts
        foreach (var script in minimalGalleries)
        {
            Debug.Log($"🗑️ Removing MinimalGallery from {script.gameObject.name}");
            Object.DestroyImmediate(script);
            removed++;
        }
        
        foreach (var script in apiTestGalleries)
        {
            Debug.Log($"🗑️ Removing APITestGallery from {script.gameObject.name}");
            Object.DestroyImmediate(script);
            removed++;
        }
        
        foreach (var script in oldImageManagers)
        {
            Debug.Log($"🗑️ Removing old ImageDisplayManager from {script.gameObject.name}");
            Object.DestroyImmediate(script);
            removed++;
        }
        
        Debug.Log($"🧹 Removed {removed} conflicting scripts");
        
        // Now add the clean script to the main display object
        GameObject mainImageDisplay = GameObject.Find("Main Image Display");
        if (mainImageDisplay == null)
        {
            // Create a new GameObject for the display
            mainImageDisplay = new GameObject("Main Image Display");
            Debug.Log("🎨 Created new Main Image Display GameObject");
        }
        
        // Ensure it has a RawImage component
        RawImage rawImage = mainImageDisplay.GetComponent<RawImage>();
        if (rawImage == null)
        {
            rawImage = mainImageDisplay.AddComponent<RawImage>();
            Debug.Log("🖼️ Added RawImage component");
        }
        
        // Add the clean manager script
        CleanImageDisplayManager cleanManager = mainImageDisplay.GetComponent<CleanImageDisplayManager>();
        if (cleanManager == null)
        {
            cleanManager = mainImageDisplay.AddComponent<CleanImageDisplayManager>();
            cleanManager.imageDisplay = rawImage;
            Debug.Log("✅ Added CleanImageDisplayManager");
        }
        
        // Set up the RawImage for fullscreen display
        RectTransform rectTransform = rawImage.GetComponent<RectTransform>();
        if (rectTransform != null)
        {
            rectTransform.anchorMin = Vector2.zero;
            rectTransform.anchorMax = Vector2.one;
            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;
            Debug.Log("🖥️ Configured RawImage for fullscreen display");
        }
        
        Debug.Log("✅ Scene cleanup complete! Only CleanImageDisplayManager remains.");
        Debug.Log("💡 Build the app now to test the single-script solution.");
        
        // Mark the scene as dirty so Unity knows to save changes
        UnityEditor.SceneManagement.EditorSceneManager.MarkSceneDirty(
            UnityEditor.SceneManagement.EditorSceneManager.GetActiveScene());
    }
}