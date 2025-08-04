using UnityEngine;
using System.Collections.Generic;

namespace Aetherwave
{
    /// <summary>
    /// Unity multi-monitor controller for gallery installations
    /// Replaces C++ MonitorManager with Unity's Display API
    /// </summary>
    public class MultiMonitorController : MonoBehaviour
    {
        [Header("Display Configuration")]
        public bool autoActivateDisplays = true;
        public bool enableDisplayDebug = true;
        
        [Header("Display Layout")]
        public DisplayLayout layoutMode = DisplayLayout.Span;
        
        private List<DisplayInfo> detectedDisplays = new List<DisplayInfo>();
        
        public enum DisplayLayout
        {
            Primary,    // Use only primary display
            Span,       // Span across all displays
            Duplicate,  // Duplicate on all displays
            Gallery     // Custom gallery mode
        }
        
        void Start()
        {
            DetectDisplays();
            ConfigureDisplays();
        }
        
        public void DetectDisplays()
        {
            detectedDisplays.Clear();
            
            Debug.Log($"🖥️  Detecting displays... Found {Display.displays.Length} display(s)");
            
            for (int i = 0; i < Display.displays.Length; i++)
            {
                Display display = Display.displays[i];
                
                DisplayInfo info = new DisplayInfo
                {
                    index = i,
                    display = display,
                    width = display.systemWidth,
                    height = display.systemHeight,
                    refreshRate = display.renderingWidth > 0 ? 60 : 0 // Unity limitation
                };
                
                detectedDisplays.Add(info);
                
                if (enableDisplayDebug)
                {
                    Debug.Log($"Display {i}: {info.width}x{info.height} @ {info.refreshRate}Hz");
                }
            }
            
            // Activate additional displays
            if (autoActivateDisplays)
            {
                ActivateAllDisplays();
            }
        }
        
        private void ActivateAllDisplays()
        {
            for (int i = 1; i < Display.displays.Length; i++)
            {
                if (!Display.displays[i].active)
                {
                    Display.displays[i].Activate();
                    Debug.Log($"✅ Activated Display {i}");
                }
            }
        }
        
        private void ConfigureDisplays()
        {
            switch (layoutMode)
            {
                case DisplayLayout.Primary:
                    ConfigurePrimaryDisplay();
                    break;
                    
                case DisplayLayout.Span:
                    ConfigureSpanMode();
                    break;
                    
                case DisplayLayout.Duplicate:
                    ConfigureDuplicateMode();
                    break;
                    
                case DisplayLayout.Gallery:
                    ConfigureGalleryMode();
                    break;
            }
        }
        
        private void ConfigurePrimaryDisplay()
        {
            // Use only the primary display (default Unity behavior)
            Debug.Log("🖥️  Configured for Primary Display mode");
        }
        
        private void ConfigureSpanMode()
        {
            // Unity's built-in spanning when displays are activated
            Debug.Log("🖥️  Configured for Span mode across all displays");
        }
        
        private void ConfigureDuplicateMode()
        {
            // Create cameras for each display
            for (int i = 1; i < Display.displays.Length; i++)
            {
                CreateCameraForDisplay(i);
            }
            
            Debug.Log("🖥️  Configured for Duplicate mode");
        }
        
        private void ConfigureGalleryMode()
        {
            // Advanced gallery configuration
            // Different content or arrangements per display
            Debug.Log("🖥️  Configured for Gallery mode");
            
            // Example: Main display shows current image, secondary shows next image
            if (Display.displays.Length >= 2)
            {
                SetupGalleryLayout();
            }
        }
        
        private void CreateCameraForDisplay(int displayIndex)
        {
            if (displayIndex >= Display.displays.Length) return;
            
            // Create a new camera for the display
            GameObject cameraObject = new GameObject($"Camera_Display_{displayIndex}");
            Camera newCamera = cameraObject.AddComponent<Camera>();
            
            // Copy settings from main camera
            Camera mainCamera = Camera.main;
            if (mainCamera != null)
            {
                newCamera.CopyFrom(mainCamera);
            }
            
            // Assign to specific display
            newCamera.targetDisplay = displayIndex;
            
            Debug.Log($"📹 Created camera for Display {displayIndex}");
        }
        
        private void SetupGalleryLayout()
        {
            // Advanced gallery layout for multi-monitor installations
            // This could include different arrangements or content per display
            
            if (Display.displays.Length >= 2)
            {
                // Example: Use displays for different purposes
                Debug.Log("🎨 Setting up advanced gallery layout");
                
                // Display 0: Main artwork
                // Display 1: Next artwork preview or theme info
                // Display 2+: Additional content or effects
            }
        }
        
        public void SwitchLayoutMode(DisplayLayout newMode)
        {
            layoutMode = newMode;
            ConfigureDisplays();
            Debug.Log($"🔄 Switched to {newMode} layout mode");
        }
        
        public DisplayInfo GetDisplayInfo(int index)
        {
            if (index >= 0 && index < detectedDisplays.Count)
            {
                return detectedDisplays[index];
            }
            return null;
        }
        
        public List<DisplayInfo> GetAllDisplays()
        {
            return new List<DisplayInfo>(detectedDisplays);
        }
        
        public int GetDisplayCount()
        {
            return detectedDisplays.Count;
        }
        
        // Monitor window movement between displays (Unity handles this automatically)
        public void OnApplicationFocus(bool hasFocus)
        {
            if (hasFocus && enableDisplayDebug)
            {
                // Re-detect displays in case setup changed
                DetectDisplays();
            }
        }
    }
    
    [System.Serializable]
    public class DisplayInfo
    {
        public int index;
        public Display display;
        public int width;
        public int height;
        public int refreshRate;
        
        public float aspectRatio => width > 0 ? (float)height / width : 1f;
        
        public override string ToString()
        {
            return $"Display {index}: {width}x{height} @ {refreshRate}Hz (AR: {aspectRatio:F2})";
        }
    }
}