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
        
        [Header("Input Configuration")]
        public KeyCode nextImageKey = KeyCode.Space;
        public KeyCode previousImageKey = KeyCode.Backspace;
        public KeyCode refreshThemeKey = KeyCode.T;
        
        private ImageDisplayManager imageDisplayManager;
        
        void Start()
        {
            Debug.Log("🎨 Aetherwave Gallery Starting...");
            InitializeGallery();
        }
        
        void Update()
        {
            HandleInput();
        }
        
        private void InitializeGallery()
        {
            // Initialize display manager
            imageDisplayManager = GetComponent<ImageDisplayManager>();
            if (imageDisplayManager == null)
            {
                imageDisplayManager = gameObject.AddComponent<ImageDisplayManager>();
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
            
            Debug.Log("✅ Aetherwave Gallery Initialized");
        }
        
        private void HandleInput()
        {
            if (Input.GetKeyDown(nextImageKey))
            {
                imageDisplayManager?.NextImage();
            }
            
            if (Input.GetKeyDown(previousImageKey))
            {
                imageDisplayManager?.PreviousImage();
            }
            
            if (Input.GetKeyDown(refreshThemeKey))
            {
                themeManager?.RefreshTheme();
            }
            
            // ESC to quit (like SDL2 implementation)
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Application.Quit();
                #if UNITY_EDITOR
                UnityEditor.EditorApplication.isPlaying = false;
                #endif
            }
        }
    }
}
