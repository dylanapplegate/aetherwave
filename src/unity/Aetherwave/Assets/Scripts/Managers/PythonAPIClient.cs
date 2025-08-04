using UnityEngine;
using UnityEngine.Networking;
using System.Collections;

namespace Aetherwave
{
    /// <summary>
    /// Unity HTTP client for Python FastAPI integration
    /// Handles theme detection and image analysis requests
    /// </summary>
    public class PythonAPIClient : MonoBehaviour
    {
        [Header("API Configuration")]
        public string apiBaseUrl = "http://localhost:8000";
        public float requestTimeout = 10f;
        
        private ThemeProfile currentTheme;
        
        void Start()
        {
            Debug.Log("🐍 Python API Client initialized");
        }
        
        public void RefreshTheme()
        {
            StartCoroutine(RequestThemeAnalysis());
        }
        
        private IEnumerator RequestThemeAnalysis()
        {
            string url = $"{apiBaseUrl}/analyze/collection-theme";
            
            using (UnityWebRequest request = UnityWebRequest.Get(url))
            {
                request.timeout = (int)requestTimeout;
                
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    Debug.Log("✅ Theme analysis received from Python API");
                    // Parse JSON response and update theme
                    // This will be implemented when Python API is ready
                }
                else
                {
                    Debug.LogWarning($"⚠️ Python API request failed: {request.error}");
                }
            }
        }
        
        public ThemeProfile GetCurrentTheme()
        {
            return currentTheme;
        }
    }
}