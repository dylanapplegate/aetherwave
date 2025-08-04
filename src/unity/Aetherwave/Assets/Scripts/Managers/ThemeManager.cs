using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

namespace Aetherwave
{
    /// <summary>
    /// Unity theme manager for content-driven aesthetic adaptation
    /// Replaces C++ ThemeManager with Unity's rendering pipeline integration
    /// </summary>
    public class ThemeManager : MonoBehaviour
    {
        [Header("Rendering Components")]
        public Camera mainCamera;
        public Volume postProcessVolume;
        
        [Header("UI Elements")]
        public UnityEngine.UI.Image backgroundPanel;
        public UnityEngine.UI.Text themeDebugText;
        
        [Header("Theme Configuration")]
        public bool enableThemeDebug = false;
        public float colorTransitionSpeed = 2f;
        
        private ThemeProfile activeTheme;
        private Vignette vignette;
        private Bloom bloom;
        private ChromaticAberration chromaticAberration;
        private ColorAdjustments colorAdjustments;
        
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
            
            // Get post-processing components
            if (postProcessVolume != null && postProcessVolume.profile != null)
            {
                postProcessVolume.profile.TryGet(out vignette);
                postProcessVolume.profile.TryGet(out bloom);
                postProcessVolume.profile.TryGet(out chromaticAberration);
                postProcessVolume.profile.TryGet(out colorAdjustments);
            }
            
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
            
            // Apply post-processing effects based on theme
            ApplyPostProcessingEffects(theme);
            
            // Update debug display
            UpdateThemeDebugDisplay();
        }
        
        private void ApplyPostProcessingEffects(ThemeProfile theme)
        {
            if (postProcessVolume == null) return;
            
            switch (theme.themeName.ToLower())
            {
                case "cyberfemme":
                    ApplyCyberfemmeEffects(theme);
                    break;
                    
                case "organic":
                    ApplyOrganicEffects(theme);
                    break;
                    
                case "tech":
                    ApplyTechEffects(theme);
                    break;
                    
                case "vintage":
                    ApplyVintageEffects(theme);
                    break;
                    
                default:
                    ApplyDefaultEffects(theme);
                    break;
            }
        }
        
        private void ApplyCyberfemmeEffects(ThemeProfile theme)
        {
            // High-intensity bloom for neon glow
            if (bloom != null)
            {
                bloom.intensity.value = 0.8f * theme.effectIntensity;
                bloom.threshold.value = 0.7f;
                bloom.active = true;
            }
            
            // Chromatic aberration for glitch aesthetic
            if (chromaticAberration != null)
            {
                chromaticAberration.intensity.value = 0.3f * theme.effectIntensity;
                chromaticAberration.active = true;
            }
            
            // Dark vignette for focus
            if (vignette != null)
            {
                vignette.intensity.value = 0.4f;
                vignette.color.value = Color.black;
                vignette.active = true;
            }
            
            // Enhanced saturation
            if (colorAdjustments != null)
            {
                colorAdjustments.saturation.value = 20f * theme.effectIntensity;
                colorAdjustments.contrast.value = 10f;
                colorAdjustments.active = true;
            }
        }
        
        private void ApplyOrganicEffects(ThemeProfile theme)
        {
            // Subtle bloom for soft glow
            if (bloom != null)
            {
                bloom.intensity.value = 0.3f * theme.effectIntensity;
                bloom.threshold.value = 0.9f;
                bloom.active = true;
            }
            
            // No chromatic aberration for natural feel
            if (chromaticAberration != null)
            {
                chromaticAberration.active = false;
            }
            
            // Soft vignette
            if (vignette != null)
            {
                vignette.intensity.value = 0.2f;
                vignette.color.value = new Color(0.8f, 0.7f, 0.6f, 1f); // Warm tone
                vignette.active = true;
            }
            
            // Slightly desaturated, warm tone
            if (colorAdjustments != null)
            {
                colorAdjustments.saturation.value = -10f;
                colorAdjustments.hueShift.value = 15f; // Warm shift
                colorAdjustments.active = true;
            }
        }
        
        private void ApplyTechEffects(ThemeProfile theme)
        {
            // Minimal bloom for clean look
            if (bloom != null)
            {
                bloom.intensity.value = 0.1f * theme.effectIntensity;
                bloom.active = theme.effectIntensity > 0.5f;
            }
            
            // Slight chromatic aberration for digital feel
            if (chromaticAberration != null)
            {
                chromaticAberration.intensity.value = 0.1f * theme.effectIntensity;
                chromaticAberration.active = theme.effectIntensity > 0.3f;
            }
            
            // High contrast, no vignette
            if (vignette != null)
            {
                vignette.active = false;
            }
            
            // High contrast, cooler tones
            if (colorAdjustments != null)
            {
                colorAdjustments.contrast.value = 20f;
                colorAdjustments.saturation.value = 0f;
                colorAdjustments.hueShift.value = -20f; // Cool shift
                colorAdjustments.active = true;
            }
        }
        
        private void ApplyVintageEffects(ThemeProfile theme)
        {
            // No bloom for vintage look
            if (bloom != null)
            {
                bloom.active = false;
            }
            
            // No chromatic aberration
            if (chromaticAberration != null)
            {
                chromaticAberration.active = false;
            }
            
            // Warm vignette
            if (vignette != null)
            {
                vignette.intensity.value = 0.3f;
                vignette.color.value = new Color(0.9f, 0.8f, 0.6f, 1f); // Sepia tone
                vignette.active = true;
            }
            
            // Desaturated, warm, lower contrast
            if (colorAdjustments != null)
            {
                colorAdjustments.saturation.value = -30f;
                colorAdjustments.contrast.value = -15f;
                colorAdjustments.hueShift.value = 30f; // Warm/sepia shift
                colorAdjustments.active = true;
            }
        }
        
        private void ApplyDefaultEffects(ThemeProfile theme)
        {
            // Disable all effects for clean display
            if (bloom != null) bloom.active = false;
            if (chromaticAberration != null) chromaticAberration.active = false;
            if (vignette != null) vignette.active = false;
            if (colorAdjustments != null) colorAdjustments.active = false;
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
            PythonAPIClient apiClient = FindObjectOfType<PythonAPIClient>();
            if (apiClient != null)
            {
                apiClient.RefreshTheme();
            }
        }
        
        public ThemeProfile GetActiveTheme()
        {
            return activeTheme;
        }
    }
}