using UnityEngine;

namespace Aetherwave
{
    /// <summary>
    /// Color structure for theme profiles
    /// </summary>
    [System.Serializable]
    public class ColorRGB
    {
        public float r, g, b;
        
        public Color ToUnityColor()
        {
            return new Color(r, g, b, 1f);
        }
    }
    
    /// <summary>
    /// Theme profile for content-driven aesthetic adaptation
    /// </summary>
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
}