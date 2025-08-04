using UnityEngine;

namespace Aetherwave
{
    /// <summary>
    /// Emergency working gallery app - handles everything in one simple script
    /// </summary>
    public class EmergencyGallery : MonoBehaviour
    {
        void Start()
        {
            Debug.Log("🚨 EmergencyGallery Started!");
            Debug.Log($"📡 API should be running on http://localhost:8000");
            Debug.Log($"🎮 Controls: ESC to quit");

            // Set camera to solid color background
            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                mainCamera.clearFlags = CameraClearFlags.SolidColor;
                mainCamera.backgroundColor = new Color(0.2f, 0.2f, 0.2f, 1f); // Light gray
                Debug.Log("✅ Camera set to light gray background");
            }
        }

        void Update()
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("👋 Quitting Emergency Gallery");
                Application.Quit();
            }

            if (Input.GetKeyDown(KeyCode.Space))
            {
                Debug.Log("🎮 SPACE pressed - Emergency gallery responding!");
            }
        }

        void OnGUI()
        {
            GUIStyle style = new GUIStyle();
            style.fontSize = 24;
            style.normal.textColor = Color.white;

            GUI.Label(new Rect(50, 50, 800, 100), "🚨 Emergency Aetherwave Gallery", style);
            GUI.Label(new Rect(50, 100, 800, 50), "✅ Unity Scripts Working", style);
            GUI.Label(new Rect(50, 150, 800, 50), "📡 API Check: http://localhost:8000", style);
            GUI.Label(new Rect(50, 200, 800, 50), "🎮 Press SPACE to test input", style);
            GUI.Label(new Rect(50, 250, 800, 50), "🎮 Press ESC to quit", style);
        }
    }
}