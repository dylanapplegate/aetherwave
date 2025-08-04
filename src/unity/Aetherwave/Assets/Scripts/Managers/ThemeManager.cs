using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using UnityEngine.UI;

namespace Aetherwave
{
    /// <summary>
    /// Unity theme manager for content-driven aesthetic adaptation
    /// Replaces C++ ThemeManager with Unity's rendering pipeline integration
    /// Simplified for Unity 6.1 compatibility - URP post-processing to be added later
    /// </summary>
    public class ThemeManager : MonoBehaviour
    {
        [Header("Rendering Components")]
        public Camera mainCamera;
        public Volume postProcessVolume;
        
        [Header("UI Elements")]
        public Image backgroundPanel;
        public Text themeDebugText;
        
        [Header("Theme Configuration")]
        public bool enableThemeDebug = false;
        public float colorTransitionSpeed = 2f;
        
        private ThemeProfile activeTheme;
        
        void Start()
        {
            InitializeThemeSystem();
        }
        
        void Update()
        {
            if (enableThemeDebug && Input.GetKeyDown(KeyCode.T))
            {
                ToggleThemeDebug();
            }
        }
        
        public void InitializeThemeSystem()
        {
            Debug.Log("🎨 Initializing Unity Theme System...");
            
            // For Unity 6.1, we'll focus on camera and UI theming first
            // URP post-processing will be added in next development phase
            
            // Set initial cyberfemme theme as fallback
            if (activeTheme == null)
            {
                ApplyFallbackTheme();
            }
            
            Debug.Log("✅ Theme System Initialized");
        }
        
        public void ApplyTheme(ThemeProfile theme)
        {
            if (theme == null) return;
            
            activeTheme = theme;
            
            Debug.Log($"🎨 Applying Theme: {theme.themeName} (confidence: {theme.confidence:F2})");
            
            // Apply camera background color
            if (mainCamera != null)
            {
                Color bgColor = theme.backgroundColor.ToUnityColor();
                mainCamera.backgroundColor = bgColor;
            }
            
            // Apply UI background color
            if (backgroundPanel != null)
            {
                Color bgColor = theme.backgroundColor.ToUnityColor();
                backgroundPanel.color = bgColor;
            }
            
            // Apply camera and UI theming based on theme type
            ApplyBasicThemeEffects(theme);
            
            // Update debug display
            UpdateThemeDebugDisplay();
        }
        
        private void ApplyBasicThemeEffects(ThemeProfile theme)
        {
            // Basic camera and UI theming for Unity 6.1 compatibility
            // URP post-processing effects will be added in next development phase
            
            if (mainCamera == null) return;
            
            switch (theme.themeName.ToLower())
            {
                case "cyberfemme":
                    mainCamera.backgroundColor = new Color(0.1f, 0.05f, 0.2f, 1f); // Dark purple
                    break;
                    
                case "organic":
                    mainCamera.backgroundColor = new Color(0.05f, 0.15f, 0.1f, 1f); // Dark green
                    break;
                    
                case "tech":
                    mainCamera.backgroundColor = new Color(0.05f, 0.1f, 0.15f, 1f); // Dark blue
                    break;
                    
                case "vintage":
                    mainCamera.backgroundColor = new Color(0.15f, 0.12f, 0.08f, 1f); // Sepia brown
                    break;
                    
                default:
                    mainCamera.backgroundColor = Color.black;
                    break;
            }
        }
        
        private void ApplyFallbackTheme()
        {
            ThemeProfile fallback = new ThemeProfile
            {
                themeName = "cyberfemme",
                confidence = 0.5f,
                primaryColor = new ColorRGB { r = 0.8f, g = 0.2f, b = 0.8f },
                accentColor = new ColorRGB { r = 0.0f, g = 1.0f, b = 1.0f },
                backgroundColor = new ColorRGB { r = 0.1f, g = 0.1f, b = 0.2f },
                transitionDuration = 1.2f,
                effectIntensity = 0.8f
            };
            
            ApplyTheme(fallback);
        }
        
        private void ToggleThemeDebug()
        {
            enableThemeDebug = !enableThemeDebug;
            
            if (themeDebugText != null)
            {
                themeDebugText.gameObject.SetActive(enableThemeDebug);
            }
            
            UpdateThemeDebugDisplay();
        }
        
        private void UpdateThemeDebugDisplay()
        {
            if (!enableThemeDebug || themeDebugText == null || activeTheme == null) return;
            
            string debugInfo = $"Theme: {activeTheme.themeName}\\n" +
                              $"Confidence: {activeTheme.confidence:F2}\\n" +
                              $"Primary: #{ColorUtility.ToHtmlStringRGB(activeTheme.primaryColor.ToUnityColor())}\\n" +
                              $"Accent: #{ColorUtility.ToHtmlStringRGB(activeTheme.accentColor.ToUnityColor())}\\n" +
                              $"Intensity: {activeTheme.effectIntensity:F2}";
            
            themeDebugText.text = debugInfo;
        }
        
        public void RefreshTheme()
        {
            PythonAPIClient apiClient = FindFirstObjectByType<PythonAPIClient>();
            if (apiClient != null)
            {
                // Request theme refresh from Python API
                Debug.Log("🔄 Refreshing theme from Python API...");
            }
            else
            {
                Debug.LogWarning("⚠️  PythonAPIClient not found for theme refresh");
            }
        }
        
        public ThemeProfile GetActiveTheme()
        {
            return activeTheme;
        }
    }
}