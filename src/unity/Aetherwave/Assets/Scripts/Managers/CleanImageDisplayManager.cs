using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.Networking;
using System.IO;

namespace Aetherwave
{
    /// <summary>
    /// Clean image gallery manager for Aetherwave
    /// Single script solution without conflicts from other gallery scripts
    /// </summary>
    public class CleanImageDisplayManager : MonoBehaviour
    {
        [Header("Display Components")]
        public RawImage imageDisplay;

        [Header("API Integration")]
        public string apiBaseUrl = "http://localhost:8000";

        private List<string> imageUrls = new List<string>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;
        private bool showDebugInfo = true;
        private bool apiConnected = false;
        private string statusMessage = "Initializing...";

        void Start()
        {
            Debug.Log("🎨 Aetherwave CleanImageDisplayManager starting...");

            // Ensure clean background - no skybox, solid color camera
            RenderSettings.skybox = null;
            DynamicGI.UpdateEnvironment();

            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                mainCamera.clearFlags = CameraClearFlags.SolidColor;
                mainCamera.backgroundColor = new Color(0.1f, 0.1f, 0.1f, 1f);
                Debug.Log("✅ Camera configured with dark background");
            }

            // Find or create RawImage component
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
                Debug.Log("✅ RawImage component found for display");
                StartCoroutine(InitializeGallery());
            }
            else
            {
                Debug.LogError("❌ No RawImage component found! Please attach RawImage to this GameObject.");
                statusMessage = "ERROR: No RawImage component";
            }
        }

        void Update()
        {
            HandleInput();
        }

        void HandleInput()
        {
            // Navigation controls
            if (!isTransitioning && apiConnected && imageUrls.Count > 0)
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

            // UI controls
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
                Debug.Log("👋 Exiting Aetherwave");
                Application.Quit();
            }
        }

        IEnumerator InitializeGallery()
        {
            statusMessage = "Connecting to API...";
            Debug.Log("📡 Connecting to Python API...");

            // Test API connection
            string healthUrl = $"{apiBaseUrl}/health";
            using (UnityWebRequest request = UnityWebRequest.Get(healthUrl))
            {
                yield return request.SendWebRequest();

                if (request.result == UnityWebRequest.Result.Success)
                {
                    apiConnected = true;
                    statusMessage = "API Connected";
                    Debug.Log("✅ Python API connection successful");
                    
                    // Load image list
                    yield return StartCoroutine(LoadImageList());
                }
                else
                {
                    apiConnected = false;
                    statusMessage = $"API Error: {request.error}";
                    Debug.LogError($"❌ Failed to connect to API: {request.error}");
                }
            }
        }

        IEnumerator LoadImageList()
        {
            statusMessage = "Loading image list...";
            Debug.Log("📂 Loading image list from API...");

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

                    statusMessage = $"Loaded {imageUrls.Count} images";
                    Debug.Log($"✅ Loaded {imageUrls.Count} images from API");

                    if (imageUrls.Count > 0)
                    {
                        yield return StartCoroutine(LoadCurrentImage());
                    }
                    else
                    {
                        statusMessage = "No images found";
                        Debug.LogWarning("⚠️ No images found in API response");
                    }
                }
                else
                {
                    statusMessage = $"Image list error: {request.error}";
                    Debug.LogError($"❌ Failed to load image list: {request.error}");
                }
            }
        }

        IEnumerator LoadCurrentImage()
        {
            if (imageUrls.Count == 0) yield break;

            isTransitioning = true;
            string imageUrl = imageUrls[currentImageIndex];
            string filename = Path.GetFileName(imageUrl);

            statusMessage = $"Loading {filename}...";
            Debug.Log($"🖼️ Loading image {currentImageIndex + 1}/{imageUrls.Count}: {filename}");

            using (UnityWebRequest request = UnityWebRequestTexture.GetTexture(imageUrl))
            {
                yield return request.SendWebRequest();

                if (request.result == UnityWebRequest.Result.Success)
                {
                    Texture2D texture = DownloadHandlerTexture.GetContent(request);
                    imageDisplay.texture = texture;

                    // Fit image to screen with proper aspect ratio
                    FitImageToScreen(texture);

                    statusMessage = $"Image {currentImageIndex + 1}/{imageUrls.Count}";
                    Debug.Log($"✅ Loaded image: {texture.width}x{texture.height}");
                }
                else
                {
                    statusMessage = $"Image load error: {request.error}";
                    Debug.LogError($"❌ Failed to load image: {request.error}");
                }
            }

            isTransitioning = false;
        }

        void FitImageToScreen(Texture2D texture)
        {
            if (texture == null || imageDisplay == null) return;

            RectTransform rectTransform = imageDisplay.GetComponent<RectTransform>();
            if (rectTransform == null) return;

            float imageAspect = (float)texture.width / texture.height;
            float screenAspect = (float)Screen.width / Screen.height;

            // Reset to full screen anchors
            rectTransform.anchorMin = Vector2.zero;
            rectTransform.anchorMax = Vector2.one;
            rectTransform.offsetMin = Vector2.zero;
            rectTransform.offsetMax = Vector2.zero;

            // Apply aspect ratio fitting
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
            style.fontStyle = FontStyle.Bold;

            // Single, clean debug overlay
            string buildTime = System.DateTime.Now.ToString("MMM dd HH:mm");
            string buildVersion = Application.version;

            string info = $"🎨 Aetherwave v{buildVersion}\n";
            info += $"🔨 Build: {buildTime}\n";
            info += $"📡 API: {(apiConnected ? "Connected" : "Disconnected")}\n";
            info += $"📸 Status: {statusMessage}\n";
            info += $"🖥️ Fullscreen: {(Screen.fullScreen ? "ON" : "OFF")}\n";
            info += $"🎮 Controls: SPACE/← →, F=fullscreen, I=debug, ESC=exit";

            // Clean background for text
            GUI.Box(new Rect(15, 15, 450, 140), "", GUI.skin.box);
            GUI.Label(new Rect(20, 20, 440, 130), info, style);
        }
    }
}