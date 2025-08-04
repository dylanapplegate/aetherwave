using UnityEngine;
using UnityEngine.UI;
using UnityEditor;
using UnityEditor.SceneManagement;

namespace Aetherwave.Editor
{
    /// <summary>
    /// Automatic Unity scene setup for Aetherwave gallery
    /// Creates UI Canvas, Image display, and attaches necessary scripts
    /// </summary>
    public class SceneSetup
    {
        [MenuItem("Aetherwave/Setup Gallery Scene")]
        public static void SetupGalleryScene()
        {
            Debug.Log("🎨 Setting up Aetherwave Gallery Scene...");

            // Create Canvas for UI
            GameObject canvasGO = new GameObject("Gallery Canvas");
            Canvas canvas = canvasGO.AddComponent<Canvas>();
            canvas.renderMode = RenderMode.ScreenSpaceOverlay;
            canvas.sortingOrder = 0;

            CanvasScaler scaler = canvasGO.AddComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.screenMatchMode = CanvasScaler.ScreenMatchMode.MatchWidthOrHeight;
            scaler.matchWidthOrHeight = 0.5f;

            canvasGO.AddComponent<GraphicRaycaster>();

            // Create main image display
            GameObject imageGO = new GameObject("Main Image Display");
            imageGO.transform.SetParent(canvasGO.transform, false);

            RectTransform imageRect = imageGO.AddComponent<RectTransform>();
            imageRect.anchorMin = Vector2.zero;
            imageRect.anchorMax = Vector2.one;
            imageRect.offsetMin = Vector2.zero;
            imageRect.offsetMax = Vector2.zero;

            RawImage rawImage = imageGO.AddComponent<RawImage>();
            rawImage.color = Color.black; // Start with black background

            // Create Gallery Manager GameObject
            GameObject galleryManagerGO = new GameObject("Aetherwave Gallery Manager");
            AetherwaveGallery gallery = galleryManagerGO.AddComponent<AetherwaveGallery>();

            // Find and assign components
            Camera mainCamera = Camera.main;
            if (mainCamera == null)
            {
                mainCamera = Object.FindFirstObjectByType<Camera>();
            }

            // Create and assign managers
            ThemeManager themeManager = galleryManagerGO.AddComponent<ThemeManager>();
            MultiMonitorController multiController = galleryManagerGO.AddComponent<MultiMonitorController>();

            // Wire up references
            gallery.mainImageDisplay = rawImage;
            gallery.galleryCamera = mainCamera;
            gallery.themeManager = themeManager;
            gallery.multiMonitorController = multiController;

            // Set camera background to black for gallery aesthetic
            if (mainCamera != null)
            {
                mainCamera.backgroundColor = Color.black;
                mainCamera.clearFlags = CameraClearFlags.SolidColor;
            }

            // Create EventSystem if it doesn't exist
            if (Object.FindFirstObjectByType<UnityEngine.EventSystems.EventSystem>() == null)
            {
                GameObject eventSystemGO = new GameObject("EventSystem");
                eventSystemGO.AddComponent<UnityEngine.EventSystems.EventSystem>();
                eventSystemGO.AddComponent<UnityEngine.EventSystems.StandaloneInputModule>();
            }

            // Mark scene as dirty and save
            EditorSceneManager.MarkSceneDirty(EditorSceneManager.GetActiveScene());

            Debug.Log("✅ Gallery Scene Setup Complete!");
            Debug.Log("📸 Main Image Display: " + rawImage.name);
            Debug.Log("🎮 Gallery Manager: " + gallery.name);
            Debug.Log("🎨 Theme Manager: " + themeManager.name);
            Debug.Log("🖥️  Multi-Monitor Controller: " + multiController.name);
            Debug.Log("🚀 Press Play to start the gallery!");

            // Select the gallery manager for easy inspection
            Selection.activeGameObject = galleryManagerGO;
        }
    }
}
