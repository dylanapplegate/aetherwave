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

        private List<Texture2D> images = new List<Texture2D>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;

        private PythonAPIClient apiClient;

        void Start()
        {
            apiClient = FindFirstObjectByType<PythonAPIClient>();
            LoadImages();

            if (images.Count > 0)
            {
                ShowImage(0);
            }
        }

        private void LoadImages()
        {
            string imagePath = Path.Combine(Application.streamingAssetsPath, imageDirectory);

            if (!Directory.Exists(imagePath))
            {
                Debug.LogWarning($"Image directory not found: {imagePath}");
                return;
            }

            string[] supportedExtensions = { "*.jpg", "*.jpeg", "*.png", "*.bmp" };

            foreach (string extension in supportedExtensions)
            {
                string[] files = Directory.GetFiles(imagePath, extension, SearchOption.AllDirectories);

                foreach (string file in files)
                {
                    StartCoroutine(LoadImageCoroutine(file));
                }
            }

            Debug.Log($"📸 Loaded {images.Count} images for display");
        }

        private IEnumerator LoadImageCoroutine(string filePath)
        {
            // Unity 6.1 compatible image loading using byte array approach
            byte[] imageData = null;

            try
            {
                imageData = File.ReadAllBytes(filePath);
            }
            catch (System.Exception e)
            {
                Debug.LogError($"Failed to read image file: {filePath} - {e.Message}");
                yield break;
            }

            if (imageData != null && imageData.Length > 0)
            {
                Texture2D texture = new Texture2D(2, 2); // Temporary size, will be resized by LoadImage

                if (texture.LoadImage(imageData))
                {
                    texture.name = Path.GetFileName(filePath);
                    images.Add(texture);
                }
                else
                {
                    Debug.LogError($"Failed to load image data: {filePath}");
                    DestroyImmediate(texture);
                }
            }

            yield return null; // Allow one frame for processing
        }

        public void NextImage()
        {
            if (isTransitioning || images.Count == 0) return;

            int nextIndex = (currentImageIndex + 1) % images.Count;
            StartCoroutine(TransitionToImage(nextIndex));
        }

        public void PreviousImage()
        {
            if (isTransitioning || images.Count == 0) return;

            int prevIndex = (currentImageIndex - 1 + images.Count) % images.Count;
            StartCoroutine(TransitionToImage(prevIndex));
        }

        private IEnumerator TransitionToImage(int targetIndex)
        {
            isTransitioning = true;

            // Get theme for transition type
            ThemeProfile currentTheme = null;
            if (apiClient != null)
            {
                currentTheme = apiClient.GetCurrentTheme();
            }

            // Apply theme-based transition
            if (currentTheme != null && currentTheme.themeName == "cyberfemme")
            {
                yield return StartCoroutine(GlitchTransition(targetIndex));
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
                    1f
                );
                imageDisplay.color = glitchColor;

                elapsedTime += Time.deltaTime;
                yield return null;
            }

            // Switch image and restore
            ShowImage(targetIndex);
            imageDisplay.transform.localScale = originalScale;
            imageDisplay.transform.localPosition = originalPosition;
            imageDisplay.color = Color.white;
        }

        private void ShowImage(int index)
        {
            if (index >= 0 && index < images.Count)
            {
                imageDisplay.texture = images[index];
                Debug.Log($"🖼️  Displaying: {images[index].name} ({index + 1}/{images.Count})");
            }
        }

        public int GetImageCount()
        {
            return images.Count;
        }

        public string GetCurrentImageName()
        {
            if (currentImageIndex >= 0 && currentImageIndex < images.Count)
            {
                return images[currentImageIndex].name;
            }
            return "No Image";
        }
    }
}
