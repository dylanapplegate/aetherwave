using UnityEngine;
using System.Collections;
using UnityEngine.Networking;

namespace Aetherwave
{
    /// <summary>
    /// Minimal Unity app to test API connectivity and basic image loading
    /// </summary>
    public class APITestGallery : MonoBehaviour
    {
        void Start()
        {
            Debug.Log("🚀 APITestGallery starting...");
            StartCoroutine(TestAPIConnectivity());
        }
        
        void Update()
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("👋 Quitting API Test Gallery");
                Application.Quit();
            }
        }
        
        IEnumerator TestAPIConnectivity()
        {
            string apiBaseUrl = "http://localhost:8000";
            
            Debug.Log("📡 Testing API connectivity...");
            
            // Test health endpoint
            using (UnityWebRequest request = UnityWebRequest.Get($"{apiBaseUrl}/health"))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    Debug.Log("✅ API Health Check: " + request.downloadHandler.text);
                }
                else
                {
                    Debug.LogError("❌ API Health Check Failed: " + request.error);
                    yield break;
                }
            }
            
            // Test image list endpoint
            using (UnityWebRequest request = UnityWebRequest.Get($"{apiBaseUrl}/images/list"))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    Debug.Log("✅ Image List: " + request.downloadHandler.text.Substring(0, Mathf.Min(200, request.downloadHandler.text.Length)) + "...");
                }
                else
                {
                    Debug.LogError("❌ Image List Failed: " + request.error);
                    yield break;
                }
            }
            
            // Test theme analysis endpoint
            string themeJson = "{\"collection_path\": \"assets/images\", \"sample_size\": 5}";
            using (UnityWebRequest request = new UnityWebRequest($"{apiBaseUrl}/analyze/collection-theme", "POST"))
            {
                byte[] bodyRaw = System.Text.Encoding.UTF8.GetBytes(themeJson);
                request.uploadHandler = new UploadHandlerRaw(bodyRaw);
                request.downloadHandler = new DownloadHandlerBuffer();
                request.SetRequestHeader("Content-Type", "application/json");
                
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    Debug.Log("✅ Theme Analysis: " + request.downloadHandler.text);
                }
                else
                {
                    Debug.LogError("❌ Theme Analysis Failed: " + request.error);
                }
            }
            
            Debug.Log("🎉 API Connectivity Test Complete!");
        }
        
        void OnGUI()
        {
            GUIStyle style = new GUIStyle();
            style.fontSize = 20;
            style.normal.textColor = Color.white;
            
            GUI.Label(new Rect(50, 50, 800, 100), "🧪 Aetherwave API Test", style);
            
            style.fontSize = 16;
            GUI.Label(new Rect(50, 100, 800, 200),
                "✅ Unity App Running\n" +
                "📡 Testing Python API Connection\n" +
                "📝 Check Console for detailed results\n" +
                "🎮 Press ESC to quit", style);
        }
    }
}
