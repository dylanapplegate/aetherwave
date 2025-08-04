using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.Networking;
using System;

namespace Aetherwave
{
    /// <summary>
    /// Unity HTTP client for Aetherwave Python FastAPI classification service
    /// Replaces C++ HTTP client with Unity's networking system
    /// </summary>
    public class PythonAPIClient : MonoBehaviour
    {
        [Header("API Configuration")]
        public string apiBaseUrl = "http://localhost:8000";
        public float themeRefreshInterval = 30f;
        
        [Header("Debug")]
        public bool enableDebugLogging = true;
        
        private ThemeProfile currentTheme;
        private float lastThemeRefresh = 0f;
        
        void Start()
        {
            Debug.Log("🔌 Python API Client Starting...");
            StartCoroutine(InitializeConnection());
        }
        
        void Update()
        {
            // Auto-refresh theme periodically
            if (Time.time - lastThemeRefresh > themeRefreshInterval)
            {
                StartCoroutine(RefreshThemeAsync());
            }
        }
        
        private IEnumerator InitializeConnection()
        {
            yield return StartCoroutine(CheckAPIHealth());
            yield return StartCoroutine(LoadInitialTheme());
        }
        
        private IEnumerator CheckAPIHealth()
        {
            string healthUrl = $"{apiBaseUrl}/health";
            
            using (UnityWebRequest request = UnityWebRequest.Get(healthUrl))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    var healthData = JsonUtility.FromJson<HealthResponse>(request.downloadHandler.text);
                    Debug.Log($"✅ Connected to Python API v{healthData.service_version}");
                }
                else
                {
                    Debug.LogWarning($"⚠️  Python API not available: {request.error}");
                    Debug.LogWarning("🔧 Make sure to run: ./scripts/dev-setup.sh");
                }
            }
        }
        
        private IEnumerator LoadInitialTheme()
        {
            yield return StartCoroutine(RefreshThemeAsync());
        }
        
        public IEnumerator RefreshThemeAsync()
        {
            string themeUrl = $"{apiBaseUrl}/analyze/collection-theme";
            
            using (UnityWebRequest request = UnityWebRequest.Get(themeUrl))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    try
                    {
                        var themeData = JsonUtility.FromJson<ThemeResponse>(request.downloadHandler.text);
                        currentTheme = themeData.theme_profile;
                        lastThemeRefresh = Time.time;
                        
                        if (enableDebugLogging)
                        {
                            Debug.Log($"🎨 Theme Updated: {currentTheme.themeName} (confidence: {currentTheme.confidence:F2})");
                        }
                        
                        // Notify theme manager
                        ThemeManager themeManager = FindObjectOfType<ThemeManager>();
                        if (themeManager != null)
                        {
                            themeManager.ApplyTheme(currentTheme);
                        }
                    }
                    catch (Exception e)
                    {
                        Debug.LogError($"Failed to parse theme response: {e.Message}");
                    }
                }
                else
                {
                    Debug.LogWarning($"Failed to fetch theme: {request.error}");
                }
            }
        }
        
        public ThemeProfile GetCurrentTheme()
        {
            return currentTheme ?? CreateFallbackTheme();
        }
        
        private ThemeProfile CreateFallbackTheme()
        {
            return new ThemeProfile
            {
                themeName = "cyberfemme",
                confidence = 0.5f,
                primaryColor = new ColorRGB { r = 0.8f, g = 0.2f, b = 0.8f }, // Magenta
                accentColor = new ColorRGB { r = 0.0f, g = 1.0f, b = 1.0f },  // Cyan
                backgroundColor = new ColorRGB { r = 0.1f, g = 0.1f, b = 0.2f }, // Dark purple
                transitionDuration = 1.2f,
                effectIntensity = 0.8f
            };
        }
        
        // Manual theme refresh for user input
        public void RefreshTheme()
        {
            StartCoroutine(RefreshThemeAsync());
        }
    }
    
    // Data structures for API responses
    [System.Serializable]
    public class HealthResponse
    {
        public bool healthy;
        public string service_version;
        public int total_images_classified;
        public int total_themes_generated;
    }
    
    [System.Serializable]
    public class ThemeResponse
    {
        public ThemeProfile theme_profile;
        public string collection_path;
        public int total_images;
        public float analysis_time;
    }
    
    [System.Serializable]
    public class ThemeProfile
    {
        public string themeName;
        public float confidence;
        public ColorRGB primaryColor;
        public ColorRGB accentColor;
        public ColorRGB backgroundColor;
        public float transitionDuration;
        public float effectIntensity;
    }
    
    [System.Serializable]
    public class ColorRGB
    {
        public float r;
        public float g;
        public float b;
        
        public Color ToUnityColor()
        {
            return new Color(r, g, b, 1f);
        }
    }
}