using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.Networking;
using System.IO;

namespace Aetherwave
{
    /// <summary>
    /// Manages image display and navigation for Aetherwave gallery
    /// Loads images from local assets and integrates with Python API for themes
    /// </summary>
    public class ImageDisplayManager : MonoBehaviour
    {
        [Header("Display Components")]
        public RawImage imageDisplay;
        
        [Header("Navigation Settings")]
        public float transitionDuration = 1.0f;
        public KeyCode nextImageKey = KeyCode.Space;
        public KeyCode previousImageKey = KeyCode.Backspace;
        public KeyCode refreshThemeKey = KeyCode.T;
        
        [Header("API Integration")]
        public string apiBaseUrl = "http://localhost:8000";
        
        private List<string> imageUrls = new List<string>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;
        private string currentTheme = "adaptive";
        
        void Start()
        {
            Debug.Log("🎨 ImageDisplayManager starting...");
            
            if (imageDisplay == null)
            {
                imageDisplay = GetComponent<RawImage>();
            }
            
            StartCoroutine(InitializeGallery());
        }
        
        void Update()
        {
            if (!isTransitioning)
            {
                if (Input.GetKeyDown(nextImageKey))
                {
                    NextImage();
                }
                else if (Input.GetKeyDown(previousImageKey))
                {
                    PreviousImage();
                }
                else if (Input.GetKeyDown(refreshThemeKey))
                {
                    StartCoroutine(RefreshTheme());
                }
            }
            
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("👋 Quitting Aetherwave Gallery");
                Application.Quit();
            }
        }
        
        IEnumerator InitializeGallery()
        {
            Debug.Log("📡 Connecting to Python API...");
            
            // Get list of available images from API
            yield return StartCoroutine(LoadImageList());
            
            if (imageUrls.Count > 0)
            {
                Debug.Log($"📸 Found {imageUrls.Count} images");
                yield return StartCoroutine(LoadCurrentImage());
                yield return StartCoroutine(RefreshTheme());
            }
            else
            {
                Debug.LogError("❌ No images found! Check API connection.");
            }
        }
        
        IEnumerator LoadImageList()
        {
            string url = $"{apiBaseUrl}/images/list";
            
            using (UnityWebRequest request = UnityWebRequest.Get(url))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    try
                    {
                        var response = JsonUtility.FromJson<ImageListResponse>(request.downloadHandler.text);
                        imageUrls.Clear();
                        
                        foreach (string imagePath in response.images)
                        {
                            // Convert local paths to API serve URLs
                            string filename = Path.GetFileName(imagePath);
                            imageUrls.Add($"{apiBaseUrl}/images/{filename}");
                        }
                        
                        Debug.Log($"✅ Loaded {imageUrls.Count} image URLs from API");
                    }
                    catch (System.Exception e)
                    {
                        Debug.LogError($"❌ Failed to parse image list: {e.Message}");
                    }
                }
                else
                {
                    Debug.LogError($"❌ Failed to load image list: {request.error}");
                }
            }
        }
        
        IEnumerator LoadCurrentImage()
        {
            if (imageUrls.Count == 0) yield break;
            
            isTransitioning = true;
            string imageUrl = imageUrls[currentImageIndex];
            
            Debug.Log($"🖼️ Loading image {currentImageIndex + 1}/{imageUrls.Count}: {Path.GetFileName(imageUrl)}");
            
            using (UnityWebRequest request = UnityWebRequestTexture.GetTexture(imageUrl))
            {
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    Texture2D texture = DownloadHandlerTexture.GetContent(request);
                    
                    if (imageDisplay != null)
                    {
                        // Fade transition
                        yield return StartCoroutine(FadeToTexture(texture));
                    }
                    
                    Debug.Log($"✅ Loaded image: {texture.width}x{texture.height}");
                }
                else
                {
                    Debug.LogError($"❌ Failed to load image: {request.error}");
                }
            }
            
            isTransitioning = false;
        }
        
        IEnumerator FadeToTexture(Texture2D newTexture)
        {
            float elapsed = 0f;
            Color originalColor = imageDisplay.color;
            
            // Fade out
            while (elapsed < transitionDuration / 2f)
            {
                elapsed += Time.deltaTime;
                float alpha = Mathf.Lerp(1f, 0f, elapsed / (transitionDuration / 2f));
                imageDisplay.color = new Color(originalColor.r, originalColor.g, originalColor.b, alpha);
                yield return null;
            }
            
            // Change texture
            imageDisplay.texture = newTexture;
            
            // Fade in
            elapsed = 0f;
            while (elapsed < transitionDuration / 2f)
            {
                elapsed += Time.deltaTime;
                float alpha = Mathf.Lerp(0f, 1f, elapsed / (transitionDuration / 2f));
                imageDisplay.color = new Color(originalColor.r, originalColor.g, originalColor.b, alpha);
                yield return null;
            }
            
            imageDisplay.color = originalColor;
        }
        
        public void NextImage()
        {
            if (imageUrls.Count == 0) return;
            
            currentImageIndex = (currentImageIndex + 1) % imageUrls.Count;
            StartCoroutine(LoadCurrentImage());
        }
        
        public void PreviousImage()
        {
            if (imageUrls.Count == 0) return;
            
            currentImageIndex = (currentImageIndex - 1 + imageUrls.Count) % imageUrls.Count;
            StartCoroutine(LoadCurrentImage());
        }
        
        IEnumerator RefreshTheme()
        {
            Debug.Log("🎨 Refreshing theme from API...");
            
            string url = $"{apiBaseUrl}/analyze/collection-theme";
            string json = "{\"collection_path\": \"assets/images\", \"sample_size\": 5}";
            
            using (UnityWebRequest request = new UnityWebRequest(url, "POST"))
            {
                byte[] bodyRaw = System.Text.Encoding.UTF8.GetBytes(json);
                request.uploadHandler = new UploadHandlerRaw(bodyRaw);
                request.downloadHandler = new DownloadHandlerBuffer();
                request.SetRequestHeader("Content-Type", "application/json");
                
                yield return request.SendWebRequest();
                
                if (request.result == UnityWebRequest.Result.Success)
                {
                    try
                    {
                        var response = JsonUtility.FromJson<ThemeResponse>(request.downloadHandler.text);
                        currentTheme = response.theme.theme_name;
                        
                        Debug.Log($"✅ Current theme: {currentTheme}");
                        Debug.Log($"🎨 Primary colors: {string.Join(", ", response.theme.primary_colors)}");
                        
                        // Apply theme to UI (basic implementation)
                        ApplyTheme(response.theme);
                    }
                    catch (System.Exception e)
                    {
                        Debug.LogError($"❌ Failed to parse theme response: {e.Message}");
                    }
                }
                else
                {
                    Debug.LogError($"❌ Failed to get theme: {request.error}");
                }
            }
        }
        
        void ApplyTheme(ThemeData theme)
        {
            // Basic theme application - set camera background color
            Camera mainCamera = Camera.main;
            if (mainCamera != null && theme.primary_colors != null && theme.primary_colors.Length > 0)
            {
                if (ColorUtility.TryParseHtmlString(theme.primary_colors[0], out Color bgColor))
                {
                    // Darken the primary color for background
                    bgColor = Color.Lerp(bgColor, Color.black, 0.8f);
                    mainCamera.backgroundColor = bgColor;
                    Debug.Log($"🎨 Applied theme background: {theme.primary_colors[0]} -> {bgColor}");
                }
            }
        }
        
        void OnGUI()
        {
            if (imageUrls.Count == 0) return;
            
            GUIStyle style = new GUIStyle();
            style.fontSize = 16;
            style.normal.textColor = Color.white;
            style.alignment = TextAnchor.UpperLeft;
            
            string info = $"🎨 Aetherwave Gallery\n";
            info += $"📸 Image: {currentImageIndex + 1}/{imageUrls.Count}\n";
            info += $"🎭 Theme: {currentTheme}\n";
            info += $"🎮 Controls: SPACE/← →, T=theme, ESC=quit";
            
            GUI.Label(new Rect(20, 20, 400, 100), info, style);
        }
    }
    
    [System.Serializable]
    public class ImageListResponse
    {
        public string[] images;
    }
    
    [System.Serializable]
    public class ThemeResponse
    {
        public ThemeData theme;
    }
    
    [System.Serializable]
    public class ThemeData
    {
        public string theme_name;
        public string[] primary_colors;
        public float confidence;
        public string mood;
    }
}