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
        public float transitionDuration = 0.5f;
        public KeyCode nextImageKey = KeyCode.Space;
        public KeyCode previousImageKey = KeyCode.Backspace;
        public KeyCode refreshThemeKey = KeyCode.T;
        public KeyCode fullscreenToggleKey = KeyCode.F;
        public KeyCode debugInfoToggleKey = KeyCode.I;
        
        [Header("API Integration")]
        public string apiBaseUrl = "http://localhost:8000";
        
        private List<string> imageUrls = new List<string>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;
        private string currentTheme = "adaptive";
        private Vector2 lastScreenSize;
        private bool showDebugInfo = true;
        
        void Start()
        {
            Debug.Log("🎨 ImageDisplayManager starting...");
            
            if (imageDisplay == null)
            {
                Debug.Log("🔍 imageDisplay is null, trying to find RawImage component...");
                imageDisplay = GetComponent<RawImage>();
                
                if (imageDisplay == null)
                {
                    Debug.LogError("❌ No RawImage found on this GameObject!");
                    // Try to find it in children
                    imageDisplay = GetComponentInChildren<RawImage>();
                    if (imageDisplay != null)
                    {
                        Debug.Log("✅ Found RawImage in children!");
                    }
                    else
                    {
                        Debug.LogError("❌ No RawImage found anywhere!");
                    }
                }
                else
                {
                    Debug.Log("✅ Found RawImage component!");
                }
            }
            else
            {
                Debug.Log("✅ imageDisplay already assigned!");
            }
            
            StartCoroutine(InitializeGallery());
            
            // Initialize screen size tracking
            lastScreenSize = new Vector2(Screen.width, Screen.height);
        }
        
        void Update()
        {
            // Check for screen size changes (monitor changes, window resizing)
            Vector2 currentScreenSize = new Vector2(Screen.width, Screen.height);
            if (currentScreenSize != lastScreenSize)
            {
                Debug.Log($"🖥️ Screen size changed: {lastScreenSize} -> {currentScreenSize}");
                lastScreenSize = currentScreenSize;
                
                // Re-apply aspect ratio if we have a current image
                if (imageDisplay != null && imageDisplay.texture != null)
                {
                    StartCoroutine(ReapplyAspectRatioAfterScreenChange());
                }
            }
            
            if (!isTransitioning)
            {
                if (Input.GetKeyDown(nextImageKey) || Input.GetKeyDown(KeyCode.RightArrow))
                {
                    NextImage();
                }
                else if (Input.GetKeyDown(previousImageKey) || Input.GetKeyDown(KeyCode.LeftArrow))
                {
                    PreviousImage();
                }
                else if (Input.GetKeyDown(refreshThemeKey))
                {
                    StartCoroutine(RefreshTheme());
                }
            }
            
            // Fullscreen toggle (works even during transitions)
            if (Input.GetKeyDown(fullscreenToggleKey))
            {
                ToggleFullscreen();
            }
            
            // Debug info toggle (works even during transitions)
            if (Input.GetKeyDown(debugInfoToggleKey))
            {
                ToggleDebugInfo();
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
            Debug.Log($"🎬 Starting fade transition to texture: {newTexture.width}x{newTexture.height}");
            Debug.Log($"🎨 Current imageDisplay component: {(imageDisplay != null ? "Valid" : "NULL")}");
            Debug.Log($"🎨 Current imageDisplay.texture: {(imageDisplay.texture != null ? $"{imageDisplay.texture.width}x{imageDisplay.texture.height}" : "NULL")}");
            
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
            
            // Change texture and fix aspect ratio immediately
            Debug.Log($"🔄 Applying new texture to imageDisplay...");
            imageDisplay.texture = newTexture;
            
            // Apply proper aspect ratio fitting
            if (newTexture != null)
            {
                ApplyProperAspectRatio(newTexture);
                
                // Extract and apply background color from the image
                ApplyImageBasedBackgroundColor(newTexture);
            }
            
            Debug.Log($"✅ Texture applied. Current texture: {(imageDisplay.texture != null ? $"{imageDisplay.texture.width}x{imageDisplay.texture.height}" : "NULL")}");
            
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
        
        void ApplyProperAspectRatio(Texture2D texture)
        {
            RectTransform rectTransform = imageDisplay.GetComponent<RectTransform>();
            if (rectTransform == null || texture == null) return;
            
            float imageAspect = (float)texture.width / texture.height;
            float screenAspect = (float)Screen.width / Screen.height;
            
            Debug.Log($"📐 Aspect ratio calculation:");
            Debug.Log($"   - Image: {texture.width}x{texture.height} (aspect: {imageAspect:F3})");
            Debug.Log($"   - Screen: {Screen.width}x{Screen.height} (aspect: {screenAspect:F3})");
            
            // Force canvas update to ensure we have correct screen dimensions
            Canvas parentCanvas = imageDisplay.GetComponentInParent<Canvas>();
            if (parentCanvas != null)
            {
                Canvas.ForceUpdateCanvases();
                LayoutRebuilder.ForceRebuildLayoutImmediate(parentCanvas.GetComponent<RectTransform>());
            }
            
            // Always reset to full screen first
            rectTransform.anchorMin = Vector2.zero;
            rectTransform.anchorMax = Vector2.one;
            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;
            
            // Force immediate layout update
            rectTransform.ForceUpdateRectTransforms();
            
            if (imageAspect > screenAspect)
            {
                // Image is wider than screen - fit to width, letterbox top/bottom
                float heightRatio = screenAspect / imageAspect;
                float verticalMargin = (1f - heightRatio) * 0.5f;
                
                rectTransform.anchorMin = new Vector2(0f, verticalMargin);
                rectTransform.anchorMax = new Vector2(1f, 1f - verticalMargin);
                
                Debug.Log($"📐 Letterboxing applied: height ratio {heightRatio:F3}, vertical margin {verticalMargin:F3}");
            }
            else
            {
                // Image is taller than screen - fit to height, pillarbox left/right  
                float widthRatio = imageAspect / screenAspect;
                float horizontalMargin = (1f - widthRatio) * 0.5f;
                
                rectTransform.anchorMin = new Vector2(horizontalMargin, 0f);
                rectTransform.anchorMax = new Vector2(1f - horizontalMargin, 1f);
                
                Debug.Log($"📐 Pillarboxing applied: width ratio {widthRatio:F3}, horizontal margin {horizontalMargin:F3}");
            }
            
            // Ensure no additional offsets and force update
            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;
            rectTransform.ForceUpdateRectTransforms();
            
            Debug.Log($"📐 Final anchors: min({rectTransform.anchorMin.x:F3}, {rectTransform.anchorMin.y:F3}) max({rectTransform.anchorMax.x:F3}, {rectTransform.anchorMax.y:F3})");
        }
        
        void ToggleFullscreen()
        {
            bool isCurrentlyFullscreen = Screen.fullScreen;
            Screen.fullScreen = !isCurrentlyFullscreen;
            
            Debug.Log($"🖥️ Fullscreen toggled: {isCurrentlyFullscreen} -> {!isCurrentlyFullscreen}");
            
            // Re-apply aspect ratio after fullscreen change if we have a current texture
            if (imageDisplay != null && imageDisplay.texture != null)
            {
                StartCoroutine(ReapplyAspectRatioAfterScreenChange());
            }
        }
        
        IEnumerator ReapplyAspectRatioAfterScreenChange()
        {
            // Wait a frame for the screen change to take effect
            yield return null;
            
            if (imageDisplay.texture is Texture2D texture)
            {
                Debug.Log("🔄 Reapplying aspect ratio after fullscreen change...");
                ApplyProperAspectRatio(texture);
            }
        }
        
        void ToggleDebugInfo()
        {
            showDebugInfo = !showDebugInfo;
            Debug.Log($"🔍 Debug info display: {(showDebugInfo ? "ON" : "OFF")}");
        }
        
        void ApplyImageBasedBackgroundColor(Texture2D texture)
        {
            if (texture == null) return;
            
            // Sample colors from the edges of the image to get a good background color
            Color backgroundColor = ExtractDominantEdgeColor(texture);
            
            // Darken and desaturate the color for a subtle background
            backgroundColor = Color.Lerp(backgroundColor, Color.black, 0.7f); // Darken significantly
            backgroundColor = Color.Lerp(backgroundColor, new Color(backgroundColor.grayscale, backgroundColor.grayscale, backgroundColor.grayscale), 0.3f); // Slightly desaturate
            
            // Apply to camera
            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                mainCamera.backgroundColor = backgroundColor;
                Debug.Log($"🎨 Applied image-based background color: {ColorUtility.ToHtmlStringRGB(backgroundColor)}");
            }
        }
        
        Color ExtractDominantEdgeColor(Texture2D texture)
        {
            // Sample colors from the edges of the image
            Color[] edgeColors = new Color[40]; // Sample 40 edge pixels
            int colorIndex = 0;
            
            int width = texture.width;
            int height = texture.height;
            
            // Sample from top and bottom edges
            for (int i = 0; i < 10 && colorIndex < edgeColors.Length; i++)
            {
                int x = (i * width) / 10;
                if (colorIndex < edgeColors.Length) edgeColors[colorIndex++] = texture.GetPixel(x, 0); // Top edge
                if (colorIndex < edgeColors.Length) edgeColors[colorIndex++] = texture.GetPixel(x, height - 1); // Bottom edge
            }
            
            // Sample from left and right edges  
            for (int i = 0; i < 10 && colorIndex < edgeColors.Length; i++)
            {
                int y = (i * height) / 10;
                if (colorIndex < edgeColors.Length) edgeColors[colorIndex++] = texture.GetPixel(0, y); // Left edge
                if (colorIndex < edgeColors.Length) edgeColors[colorIndex++] = texture.GetPixel(width - 1, y); // Right edge
            }
            
            // Calculate average color
            float r = 0, g = 0, b = 0;
            for (int i = 0; i < colorIndex; i++)
            {
                r += edgeColors[i].r;
                g += edgeColors[i].g;
                b += edgeColors[i].b;
            }
            
            return new Color(r / colorIndex, g / colorIndex, b / colorIndex, 1f);
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
            // Theme data is available for future use
            // Background color is now set dynamically based on each image
            Debug.Log($"🎨 Theme applied: {theme.theme_name} with colors: {string.Join(", ", theme.primary_colors)}");
        }
        
        void OnGUI()
        {
            if (imageUrls.Count == 0 || !showDebugInfo) return;
            
            GUIStyle style = new GUIStyle();
            style.fontSize = 16;
            style.normal.textColor = Color.white;
            style.alignment = TextAnchor.UpperLeft;
            
            string info = $"🎨 Aetherwave Gallery\n";
            info += $"📸 Image: {currentImageIndex + 1}/{imageUrls.Count}\n";
            info += $"🎭 Theme: {currentTheme}\n";
            info += $"🖥️ Fullscreen: {(Screen.fullScreen ? "ON" : "OFF")}\n";
            info += $"🔍 Debug Info: {(showDebugInfo ? "ON" : "OFF")}\n";
            info += $"🎮 Controls: SPACE/← →, T=theme, F=fullscreen, I=debug, ESC=quit";
            
            GUI.Label(new Rect(20, 20, 450, 120), info, style);
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