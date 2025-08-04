using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;
using System.IO;
using System.Collections;

namespace Aetherwave
{
    /// <summary>
    /// Unity replacement for C++ DisplayEngine and ImageManager
    /// Handles image loading, display, and transitions with hardware acceleration
    /// </summary>
    public class ImageDisplayManager : MonoBehaviour
    {
        [Header("Image Display")]
        public RawImage imageDisplay;
        public float transitionDuration = 1.0f;
        
        [Header("Image Sources")]
        public string imageDirectory = "Images";
        
        [Header("Performance")]
        public int maxTextureSize = 2048;
        public bool enableImagePreloading = true;
        
        private List<Texture2D> images = new List<Texture2D>();
        private List<string> imagePaths = new List<string>();
        public int currentImageIndex = 0;
        public bool isTransitioning = false;
        
        private PythonAPIClient apiClient;
        private ThemeManager themeManager;
        
        void Start()
        {
            apiClient = FindObjectOfType<PythonAPIClient>();
            themeManager = FindObjectOfType<ThemeManager>();
            
            LoadImages();
            
            if (images.Count > 0)
            {
                ShowImage(0);
            }
            else
            {
                Debug.LogWarning("⚠️  No images found. Make sure images are in StreamingAssets/Images/");
            }
        }
        
        private void LoadImages()
        {
            // Clear existing images
            images.Clear();
            imagePaths.Clear();
            
            // Use StreamingAssets for runtime access
            string imagePath = Path.Combine(Application.streamingAssetsPath, imageDirectory);
            
            // Also check the main assets folder path (for development)
            if (!Directory.Exists(imagePath))
            {
                string alternativePath = Path.Combine(Application.dataPath, "StreamingAssets", imageDirectory);
                if (Directory.Exists(alternativePath))
                {
                    imagePath = alternativePath;
                }
                else
                {
                    Debug.LogWarning($"📁 Image directory not found: {imagePath}");
                    Debug.LogWarning("💡 Make sure to copy images to StreamingAssets/Images/ folder");
                    return;
                }
            }
            
            string[] supportedExtensions = { "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tga" };
            
            foreach (string extension in supportedExtensions)
            {
                string[] files = Directory.GetFiles(imagePath, extension, SearchOption.AllDirectories);
                
                foreach (string file in files)
                {
                    imagePaths.Add(file);
                }
            }
            
            Debug.Log($"📸 Found {imagePaths.Count} image files");
            
            // Load images
            if (enableImagePreloading)
            {
                StartCoroutine(PreloadAllImages());
            }
            else
            {
                // Load just the first image for immediate display
                if (imagePaths.Count > 0)
                {
                    StartCoroutine(LoadImageCoroutine(imagePaths[0], 0));
                }
            }
        }
        
        private IEnumerator PreloadAllImages()
        {
            for (int i = 0; i < imagePaths.Count; i++)
            {
                yield return StartCoroutine(LoadImageCoroutine(imagePaths[i], i));
                
                // Yield every few images to prevent frame drops
                if (i % 3 == 0)
                {
                    yield return new WaitForEndOfFrame();
                }
            }
            
            Debug.Log($"✅ Preloaded {images.Count} images for display");
        }
        
        private IEnumerator LoadImageCoroutine(string filePath, int index)
        {
            using (var www = new WWW("file://" + filePath))
            {
                yield return www;
                
                if (string.IsNullOrEmpty(www.error))
                {
                    Texture2D texture = www.texture;
                    
                    // Resize texture if too large
                    if (texture.width > maxTextureSize || texture.height > maxTextureSize)
                    {
                        texture = ResizeTexture(texture, maxTextureSize);
                    }
                    
                    texture.name = Path.GetFileName(filePath);
                    
                    // Ensure list is large enough
                    while (images.Count <= index)
                    {
                        images.Add(null);
                    }
                    
                    images[index] = texture;
                }
                else
                {
                    Debug.LogError($"Failed to load image: {filePath} - {www.error}");
                }
            }
        }
        
        private Texture2D ResizeTexture(Texture2D original, int maxSize)
        {
            int newWidth = original.width;
            int newHeight = original.height;
            
            if (newWidth > maxSize || newHeight > maxSize)
            {
                float ratio = Mathf.Min((float)maxSize / newWidth, (float)maxSize / newHeight);
                newWidth = Mathf.RoundToInt(newWidth * ratio);
                newHeight = Mathf.RoundToInt(newHeight * ratio);
            }
            
            RenderTexture rt = RenderTexture.GetTemporary(newWidth, newHeight);
            Graphics.Blit(original, rt);
            
            RenderTexture previous = RenderTexture.active;
            RenderTexture.active = rt;
            
            Texture2D resized = new Texture2D(newWidth, newHeight);
            resized.ReadPixels(new Rect(0, 0, newWidth, newHeight), 0, 0);
            resized.Apply();
            
            RenderTexture.active = previous;
            RenderTexture.ReleaseTemporary(rt);
            
            return resized;
        }
        
        public void NextImage()
        {
            if (isTransitioning || images.Count == 0) return;
            
            int nextIndex = (currentImageIndex + 1) % imagePaths.Count;
            StartCoroutine(TransitionToImage(nextIndex));
        }
        
        public void PreviousImage()
        {
            if (isTransitioning || images.Count == 0) return;
            
            int prevIndex = (currentImageIndex - 1 + imagePaths.Count) % imagePaths.Count;
            StartCoroutine(TransitionToImage(prevIndex));
        }
        
        private IEnumerator TransitionToImage(int targetIndex)
        {
            isTransitioning = true;
            
            // Load image if not preloaded
            if (targetIndex < images.Count && images[targetIndex] == null)
            {
                yield return StartCoroutine(LoadImageCoroutine(imagePaths[targetIndex], targetIndex));
            }
            
            // Get theme for transition type
            ThemeProfile currentTheme = null;
            if (apiClient != null)
            {
                currentTheme = apiClient.GetCurrentTheme();
            }
            
            // Apply theme-based transition
            if (currentTheme != null)
            {
                switch (currentTheme.themeName.ToLower())
                {
                    case "cyberfemme":
                        yield return StartCoroutine(GlitchTransition(targetIndex));
                        break;
                    case "organic":
                        yield return StartCoroutine(SoftFadeTransition(targetIndex));
                        break;
                    case "tech":
                        yield return StartCoroutine(PixelTransition(targetIndex));
                        break;
                    case "vintage":
                        yield return StartCoroutine(DissolveTransition(targetIndex));
                        break;
                    default:
                        yield return StartCoroutine(FadeTransition(targetIndex));
                        break;
                }
            }
            else
            {
                yield return StartCoroutine(FadeTransition(targetIndex));
            }
            
            currentImageIndex = targetIndex;
            isTransitioning = false;
        }
        
        private IEnumerator FadeTransition(int targetIndex)
        {
            float elapsedTime = 0f;
            Color startColor = imageDisplay.color;
            
            // Fade out
            while (elapsedTime < transitionDuration / 2)
            {
                float alpha = Mathf.Lerp(1f, 0f, elapsedTime / (transitionDuration / 2));
                imageDisplay.color = new Color(startColor.r, startColor.g, startColor.b, alpha);
                elapsedTime += Time.deltaTime;
                yield return null;
            }
            
            // Switch image
            ShowImage(targetIndex);
            
            // Fade in
            elapsedTime = 0f;
            while (elapsedTime < transitionDuration / 2)
            {
                float alpha = Mathf.Lerp(0f, 1f, elapsedTime / (transitionDuration / 2));
                imageDisplay.color = new Color(startColor.r, startColor.g, startColor.b, alpha);
                elapsedTime += Time.deltaTime;
                yield return null;
            }
            
            imageDisplay.color = startColor;
        }
        
        private IEnumerator GlitchTransition(int targetIndex)
        {
            // Cyberfemme glitch effect - quick digital transition
            float glitchDuration = 0.3f;
            float elapsedTime = 0f;
            
            Vector3 originalScale = imageDisplay.transform.localScale;
            Vector3 originalPosition = imageDisplay.transform.localPosition;
            Color originalColor = imageDisplay.color;
            
            while (elapsedTime < glitchDuration)
            {
                // Add glitch effects
                float glitchIntensity = Random.Range(0.95f, 1.05f);
                imageDisplay.transform.localScale = originalScale * glitchIntensity;
                
                float positionOffset = Random.Range(-2f, 2f);
                imageDisplay.transform.localPosition = originalPosition + Vector3.right * positionOffset;
                
                // Color channel separation effect
                Color glitchColor = new Color(
                    Random.Range(0.8f, 1.2f),
                    Random.Range(0.8f, 1.2f), 
                    Random.Range(0.8f, 1.2f),
                    originalColor.a
                );
                imageDisplay.color = glitchColor;
                
                elapsedTime += Time.deltaTime;
                yield return null;
            }
            
            // Quick image switch
            ShowImage(targetIndex);
            
            // Brief static effect
            for (int i = 0; i < 3; i++)
            {
                imageDisplay.color = Color.white;
                yield return new WaitForSeconds(0.02f);
                imageDisplay.color = originalColor;
                yield return new WaitForSeconds(0.02f);
            }
            
            // Restore original state
            imageDisplay.transform.localScale = originalScale;
            imageDisplay.transform.localPosition = originalPosition;
            imageDisplay.color = originalColor;
        }
        
        private IEnumerator SoftFadeTransition(int targetIndex)
        {
            // Organic soft fade with eased timing
            float elapsedTime = 0f;
            Color startColor = imageDisplay.color;
            
            while (elapsedTime < transitionDuration)
            {
                float t = elapsedTime / transitionDuration;
                
                // Ease in-out curve for organic feel
                float easedT = Mathf.SmoothStep(0f, 1f, t);
                
                if (t < 0.5f)
                {
                    // Fade out with ease
                    float alpha = Mathf.Lerp(1f, 0f, easedT * 2f);
                    imageDisplay.color = new Color(startColor.r, startColor.g, startColor.b, alpha);
                }
                else if (t == 0.5f)
                {
                    // Switch image at midpoint
                    ShowImage(targetIndex);
                }
                else
                {
                    // Fade in with ease
                    float alpha = Mathf.Lerp(0f, 1f, (easedT - 0.5f) * 2f);
                    imageDisplay.color = new Color(startColor.r, startColor.g, startColor.b, alpha);
                }
                
                elapsedTime += Time.deltaTime;
                yield return null;
            }
            
            imageDisplay.color = startColor;
        }
        
        private IEnumerator PixelTransition(int targetIndex)
        {
            // Tech-style instant transition with brief digital effect
            Vector3 originalScale = imageDisplay.transform.localScale;
            
            // Brief scale pulse
            imageDisplay.transform.localScale = originalScale * 1.02f;
            yield return new WaitForSeconds(0.05f);
            
            // Instant switch
            ShowImage(targetIndex);
            
            // Return to normal
            imageDisplay.transform.localScale = originalScale;
        }
        
        private IEnumerator DissolveTransition(int targetIndex)
        {
            // Vintage dissolve effect
            float dissolveTime = transitionDuration * 0.8f;
            Color originalColor = imageDisplay.color;
            
            // Gradual dissolve
            for (float t = 0; t < dissolveTime; t += Time.deltaTime)
            {
                float alpha = Mathf.Lerp(1f, 0f, t / dissolveTime);
                imageDisplay.color = new Color(originalColor.r, originalColor.g, originalColor.b, alpha);
                yield return null;
            }
            
            ShowImage(targetIndex);
            
            // Gradual appear
            for (float t = 0; t < dissolveTime; t += Time.deltaTime)
            {
                float alpha = Mathf.Lerp(0f, 1f, t / dissolveTime);
                imageDisplay.color = new Color(originalColor.r, originalColor.g, originalColor.b, alpha);
                yield return null;
            }
            
            imageDisplay.color = originalColor;
        }
        
        private void ShowImage(int index)
        {
            if (index >= 0 && index < images.Count && images[index] != null)
            {
                imageDisplay.texture = images[index];
                Debug.Log($"🖼️  Displaying: {images[index].name} ({index + 1}/{imagePaths.Count})");
            }
            else if (index >= 0 && index < imagePaths.Count)
            {
                // Load image on demand
                StartCoroutine(LoadAndShowImage(index));
            }
        }
        
        private IEnumerator LoadAndShowImage(int index)
        {
            yield return StartCoroutine(LoadImageCoroutine(imagePaths[index], index));
            
            if (index < images.Count && images[index] != null)
            {
                imageDisplay.texture = images[index];
                Debug.Log($"🖼️  Loaded and displaying: {images[index].name} ({index + 1}/{imagePaths.Count})");
            }
        }
        
        public void ReloadImages()
        {
            LoadImages();
            
            if (images.Count > 0)
            {
                currentImageIndex = 0;
                ShowImage(0);
            }
        }
        
        public int GetImageCount()
        {
            return imagePaths.Count;
        }
        
        public string GetCurrentImageName()
        {
            if (currentImageIndex >= 0 && currentImageIndex < imagePaths.Count)
            {
                return Path.GetFileName(imagePaths[currentImageIndex]);
            }
            return "No Image";
        }
        
        public string GetCurrentImagePath()
        {
            if (currentImageIndex >= 0 && currentImageIndex < imagePaths.Count)
            {
                return imagePaths[currentImageIndex];
            }
            return "";
        }
    }
}