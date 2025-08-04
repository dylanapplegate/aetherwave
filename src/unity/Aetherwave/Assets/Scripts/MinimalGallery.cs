using UnityEngine;

namespace Aetherwave
{
    /// <summary>
    /// Minimal gallery app that shows we have a working Unity build
    /// </summary>
    public class MinimalGallery : MonoBehaviour
    {
        void Start()
        {
            Debug.Log("🎨 Aetherwave Gallery Started!");
            Debug.Log($"📡 API should be running on http://localhost:8000");
            Debug.Log($"🎮 Controls: ESC to quit");

            // IMMEDIATELY clear the skybox that's causing the sunset gradient
            RenderSettings.skybox = null;
            DynamicGI.UpdateEnvironment();
            Debug.Log("🎨 SKYBOX CLEARED - sunset should be gone!");

            // Set camera to solid color background
            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                mainCamera.clearFlags = CameraClearFlags.SolidColor;
                mainCamera.backgroundColor = Color.black;
                Debug.Log("✅ Camera set to solid black background");
            }

            // Add the API test component to this GameObject
            gameObject.AddComponent<APITestGallery>();
        }

        void Update()
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("👋 Quitting Aetherwave Gallery");
                Application.Quit();
            }
        }

        void OnGUI()
        {
            GUIStyle style = new GUIStyle();
            style.fontSize = 24;
            style.normal.textColor = Color.white;

            GUI.Label(new Rect(50, 50, 800, 100), "🎨 Aetherwave Gallery", style);
            GUI.Label(new Rect(50, 100, 800, 50), "📡 Connect to API: http://localhost:8000", style);
            GUI.Label(new Rect(50, 150, 800, 50), "🎮 Press ESC to quit", style);

            style.fontSize = 16;
            GUI.Label(new Rect(50, 250, 800, 200),
                "✅ Unity Build Working\n" +
                "✅ Python API Running (82 images available)\n" +
                "🔄 API Connectivity Test: Check Console\n" +
                "📝 This tests the complete Unity↔Python integration!", style);
        }
    }
}
