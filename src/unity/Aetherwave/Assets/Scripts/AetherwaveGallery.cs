using UnityEngine;
using UnityEngine.UI;
using System.Collections;

namespace Aetherwave
{
    /// <summary>
    /// Main gallery controller for Aetherwave cyberfemme art display
    /// Replaces SDL2 C++ implementation with Unity-based rendering
    /// </summary>
    public class AetherwaveGallery : MonoBehaviour
    {
        [Header("Display Configuration")]
        public RawImage mainImageDisplay;
        public Camera galleryCamera;
        
        [Header("Theme System")]
        public ThemeManager themeManager;
        
        [Header("Multi-Monitor")]
        public MultiMonitorController multiMonitorController;
        
        [Header("API Integration")]
        public PythonAPIClient apiClient;
        
        [Header("Input Configuration")]
        public KeyCode nextImageKey = KeyCode.Space;
        public KeyCode previousImageKey = KeyCode.Backspace;
        public KeyCode refreshThemeKey = KeyCode.T;
        public KeyCode toggleDebugKey = KeyCode.F1;
        
        [Header("UI Elements")]
        public Text statusText;
        public Text imageInfoText;
        public GameObject debugPanel;
        
        private ImageDisplayManager imageDisplayManager;
        private bool showDebugInfo = false;
        
        void Start()
        {
            Debug.Log("🎨 Aetherwave Gallery Starting...");
            InitializeGallery();
        }
        
        void Update()
        {
            HandleInput();
            UpdateUI();
        }
        
        private void InitializeGallery()
        {
            // Initialize display manager
            imageDisplayManager = GetComponent<ImageDisplayManager>();
            if (imageDisplayManager == null)
            {
                imageDisplayManager = gameObject.AddComponent<ImageDisplayManager>();
            }
            
            // Set up display manager
            if (imageDisplayManager != null && mainImageDisplay != null)
            {
                imageDisplayManager.imageDisplay = mainImageDisplay;
            }
            
            // Initialize theme system
            if (themeManager != null)
            {
                themeManager.InitializeThemeSystem();
            }
            
            // Initialize multi-monitor support
            if (multiMonitorController != null)
            {
                multiMonitorController.DetectDisplays();
            }
            
            // Initialize API client
            if (apiClient != null)
            {
                Debug.Log("🔌 Python API Client ready for theme integration");
            }
            else
            {
                Debug.LogWarning("⚠️  Python API Client not assigned - theme system will use fallback");
            }
            
            // Set up UI
            if (debugPanel != null)
            {
                debugPanel.SetActive(false);
            }
            
            Debug.Log("✅ Aetherwave Gallery Initialized");
            
            // Display startup message
            UpdateStatusText("Aetherwave Gallery Ready");
        }
        
        private void HandleInput()
        {
            // Image navigation
            if (Input.GetKeyDown(nextImageKey))
            {
                imageDisplayManager?.NextImage();
                UpdateStatusText("Next Image");
            }
            
            if (Input.GetKeyDown(previousImageKey))
            {
                imageDisplayManager?.PreviousImage();
                UpdateStatusText("Previous Image");
            }
            
            // Theme controls
            if (Input.GetKeyDown(refreshThemeKey))
            {
                themeManager?.RefreshTheme();
                UpdateStatusText("Theme Refreshed");
            }
            
            // Debug toggle
            if (Input.GetKeyDown(toggleDebugKey))
            {
                ToggleDebugInfo();
            }
            
            // ESC to quit (like SDL2 implementation)
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Debug.Log("🚪 Aetherwave Gallery Exiting...");
                Application.Quit();
                #if UNITY_EDITOR
                UnityEditor.EditorApplication.isPlaying = false;
                #endif
            }
            
            // Arrow key alternatives
            if (Input.GetKeyDown(KeyCode.RightArrow) || Input.GetKeyDown(KeyCode.DownArrow))
            {
                imageDisplayManager?.NextImage();
            }
            
            if (Input.GetKeyDown(KeyCode.LeftArrow) || Input.GetKeyDown(KeyCode.UpArrow))
            {
                imageDisplayManager?.PreviousImage();
            }
        }
        
        private void UpdateUI()
        {
            // Update image info
            if (imageInfoText != null && imageDisplayManager != null)
            {
                string imageInfo = $"{imageDisplayManager.GetCurrentImageName()}\\n" +
                                  $"Image {imageDisplayManager.currentImageIndex + 1} of {imageDisplayManager.GetImageCount()}";
                imageInfoText.text = imageInfo;
            }
        }
        
        private void ToggleDebugInfo()
        {
            showDebugInfo = !showDebugInfo;
            
            if (debugPanel != null)
            {
                debugPanel.SetActive(showDebugInfo);
            }
            
            // Toggle theme debug
            if (themeManager != null)
            {
                themeManager.enableThemeDebug = showDebugInfo;
            }
            
            UpdateStatusText(showDebugInfo ? "Debug Info ON" : "Debug Info OFF");
        }
        
        private void UpdateStatusText(string message)
        {
            if (statusText != null)
            {
                statusText.text = message;
                
                // Auto-clear status after 2 seconds
                StartCoroutine(ClearStatusAfterDelay(2f));
            }
        }
        
        private IEnumerator ClearStatusAfterDelay(float delay)
        {
            yield return new WaitForSeconds(delay);
            
            if (statusText != null)
            {
                statusText.text = "";
            }
        }
        
        // Public methods for external control
        public void SetImageDirectory(string directory)
        {
            if (imageDisplayManager != null)
            {
                imageDisplayManager.imageDirectory = directory;
                imageDisplayManager.ReloadImages();
            }
        }
        
        public void SetTransitionDuration(float duration)
        {
            if (imageDisplayManager != null)
            {
                imageDisplayManager.transitionDuration = duration;
            }
        }
        
        public void NextImage()
        {
            imageDisplayManager?.NextImage();
        }
        
        public void PreviousImage()
        {
            imageDisplayManager?.PreviousImage();
        }
        
        public void RefreshTheme()
        {
            themeManager?.RefreshTheme();
        }
        
        // Gallery status information
        public int GetImageCount()
        {
            return imageDisplayManager?.GetImageCount() ?? 0;
        }
        
        public string GetCurrentImageName()
        {
            return imageDisplayManager?.GetCurrentImageName() ?? "No Image";
        }
        
        public bool IsTransitioning()
        {
            return imageDisplayManager?.isTransitioning ?? false;
        }
        
        public string GetActiveThemeName()
        {
            var theme = themeManager?.GetActiveTheme();
            return theme?.themeName ?? "Unknown";
        }
        
        void OnApplicationQuit()
        {
            Debug.Log("🎨 Aetherwave Gallery Shutting Down...");
        }
    }
}