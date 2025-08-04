using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.Networking;
using System.IO;

namespace Aetherwave
{
    /// <summary>
    /// Simple image gallery manager for Aetherwave
    /// Focuses on basic image display without complex background effects
    /// </summary>
    public class ImageDisplayManager : MonoBehaviour
    {
        [Header("Display Components")]
        public RawImage imageDisplay;

        [Header("API Integration")]
        public string apiBaseUrl = "http://localhost:8000";

        private List<string> imageUrls = new List<string>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;
        private bool showDebugInfo = true;

        void Start()
        {
            Debug.Log("🎨 ImageDisplayManager starting (simplified version)...");

            // Simple camera setup - just dark gray background
            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                mainCamera.clearFlags = CameraClearFlags.SolidColor;
                mainCamera.backgroundColor = new Color(0.15f, 0.15f, 0.15f, 1f);
                Debug.Log("✅ Camera set to dark gray background");
            }

            // Find the RawImage component
            if (imageDisplay == null)
            {
                imageDisplay = GetComponent<RawImage>();
                if (imageDisplay == null)
                {
                    imageDisplay = GetComponentInChildren<RawImage>();
                }
            }

            if (imageDisplay != null)
            {
                Debug.Log("✅ Found RawImage component for display");
                StartCoroutine(InitializeGallery());
            }
            else
            {
                Debug.LogError("❌ No RawImage component found!");
            }
        }

        void Update()
        {
            // Simple keyboard controls
            if (!isTransitioning)
            {
                if (Input.GetKeyDown(KeyCode.Space) || Input.GetKeyDown(KeyCode.RightArrow))
                {
                    NextImage();
                }
                else if (Input.GetKeyDown(KeyCode.Backspace) || Input.GetKeyDown(KeyCode.LeftArrow))
                {
                    PreviousImage();
                }
            }

            if (Input.GetKeyDown(KeyCode.F))
            {
                Screen.fullScreen = !Screen.fullScreen;
                Debug.Log($"🖥️ Fullscreen: {Screen.fullScreen}");
            }

            if (Input.GetKeyDown(KeyCode.I))
            {
                showDebugInfo = !showDebugInfo;
                Debug.Log($"🔍 Debug info: {showDebugInfo}");
            }

            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("👋 Quitting gallery");
                Application.Quit();
            }
        }

        IEnumerator InitializeGallery()
        {
            Debug.Log("📡 Loading image list from API...");

            string url = $"{apiBaseUrl}/images/list";
            using (UnityWebRequest request = UnityWebRequest.Get(url))
            {
                yield return request.SendWebRequest();

                if (request.result == UnityWebRequest.Result.Success)
                {
                    var response = JsonUtility.FromJson<ImageListResponse>(request.downloadHandler.text);
                    imageUrls.Clear();

                    foreach (string imagePath in response.images)
                    {
                        string filename = Path.GetFileName(imagePath);
                        imageUrls.Add($"{apiBaseUrl}/images/{filename}");
                    }

                    Debug.Log($"✅ Loaded {imageUrls.Count} images from API");

                    if (imageUrls.Count > 0)
                    {
                        yield return StartCoroutine(LoadCurrentImage());
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
                    imageDisplay.texture = texture;

                    // Simple aspect ratio fitting
                    FitImageToScreen(texture);

                    Debug.Log($"✅ Loaded image: {texture.width}x{texture.height}");
                }
                else
                {
                    Debug.LogError($"❌ Failed to load image: {request.error}");
                }
            }

            isTransitioning = false;
        }

        void FitImageToScreen(Texture2D texture)
        {
            if (texture == null) return;

            RectTransform rectTransform = imageDisplay.GetComponent<RectTransform>();
            if (rectTransform == null) return;

            float imageAspect = (float)texture.width / texture.height;
            float screenAspect = (float)Screen.width / Screen.height;

            // Reset to full screen
            rectTransform.anchorMin = Vector2.zero;
            rectTransform.anchorMax = Vector2.one;
            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;

            if (imageAspect > screenAspect)
            {
                // Image wider than screen - fit width, letterbox height
                float heightRatio = screenAspect / imageAspect;
                float margin = (1f - heightRatio) * 0.5f;
                rectTransform.anchorMin = new Vector2(0f, margin);
                rectTransform.anchorMax = new Vector2(1f, 1f - margin);
            }
            else
            {
                // Image taller than screen - fit height, pillarbox width
                float widthRatio = imageAspect / screenAspect;
                float margin = (1f - widthRatio) * 0.5f;
                rectTransform.anchorMin = new Vector2(margin, 0f);
                rectTransform.anchorMax = new Vector2(1f - margin, 1f);
            }

            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;
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

        void OnGUI()
        {
            if (!showDebugInfo) return;

            GUIStyle style = new GUIStyle();
            style.fontSize = 16;
            style.normal.textColor = Color.white;

            // Get build time from Application.buildGUID or current time as fallback
            string buildTime = System.DateTime.Now.ToString("MMM dd HH:mm");
            string buildVersion = Application.version;

            string info = $"🎨 Aetherwave Gallery v{buildVersion}\n";
            info += $"🔨 Build: {buildTime} (Updated Aug 4)\n";
            info += $"📸 Image: {(imageUrls.Count > 0 ? $"{currentImageIndex + 1}/{imageUrls.Count}" : "Loading...")}\n";
            info += $"🖥️ Fullscreen: {(Screen.fullScreen ? "ON" : "OFF")}\n";
            info += $"🎮 Controls: SPACE/← →, F=fullscreen, I=debug, ESC=quit";

            GUI.Label(new Rect(20, 20, 400, 120), info, style);
        }
    }

    [System.Serializable]
    public class ImageListResponse
    {
        public string[] images;
    }
}