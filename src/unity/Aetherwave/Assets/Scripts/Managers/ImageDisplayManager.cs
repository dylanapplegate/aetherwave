using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using System.Collections.Generic;
using System.Collections;
using System;

namespace Aetherwave
{
    /// <summary>
    /// Unity replacement for C++ DisplayEngine and ImageManager
    /// Loads images from Python API server instead of local files
    /// Handles image display and transitions with hardware acceleration
    /// </summary>
    public class ImageDisplayManager : MonoBehaviour
    {
        [Header("Image Display")]
        public RawImage imageDisplay;
        public float transitionDuration = 1.0f;

        [Header("API Configuration")]
        public string apiBaseUrl = "http://localhost:8000";

        [System.Serializable]
        public class ImageListResponse
        {
            public string[] images;
            public int count;
            public string base_url;
        }

        private List<string> imageFilenames = new List<string>();
        private Dictionary<string, Texture2D> imageCache = new Dictionary<string, Texture2D>();
        private int currentImageIndex = 0;
        private bool isTransitioning = false;
        private bool imagesLoaded = false;

        private PythonAPIClient apiClient;

        void Start()
        {
            apiClient = FindFirstObjectByType<PythonAPIClient>();

            if (imageDisplay == null)
            {
                Debug.LogError("ImageDisplayManager: No RawImage component assigned!");
                return;
            }

            StartCoroutine(LoadImageList());
        }

        private IEnumerator LoadImageList()
        {
            Debug.Log("🔍 Loading image list from API...");

            string url = $"{apiBaseUrl}/images/list";

            using (UnityWebRequest www = UnityWebRequest.Get(url))
            {
                yield return www.SendWebRequest();

                if (www.result != UnityWebRequest.Result.Success)
                {
                    Debug.LogError($"Failed to load image list: {www.error}");
                    yield break;
                }

                try
                {
                    ImageListResponse response = JsonUtility.FromJson<ImageListResponse>(www.downloadHandler.text);
                    imageFilenames.Clear();
                    imageFilenames.AddRange(response.images);

                    Debug.Log($"📸 Loaded {imageFilenames.Count} images from API");

                    if (imageFilenames.Count > 0)
                    {
                        imagesLoaded = true;
                        yield return StartCoroutine(LoadAndShowImage(0));
                    }
                    else
                    {
                        Debug.LogWarning("No images found in API response");
                    }
                }
                catch (Exception e)
                {
                    Debug.LogError($"Failed to parse image list response: {e.Message}");
                }
            }
        }

        private IEnumerator LoadAndShowImage(int index)
        {
            if (index < 0 || index >= imageFilenames.Count)
            {
                Debug.LogWarning($"Invalid image index: {index}");
                yield break;
            }

            string filename = imageFilenames[index];

            // Check cache first
            if (imageCache.ContainsKey(filename))
            {
                imageDisplay.texture = imageCache[filename];
                currentImageIndex = index;
                Debug.Log($"🖼️  Showing cached image {index + 1}/{imageFilenames.Count}: {filename}");
                yield break;
            }

            Debug.Log($"⬇️  Loading image {index + 1}/{imageFilenames.Count}: {filename}");

            string url = $"{apiBaseUrl}/images/{filename}";

            using (UnityWebRequest www = UnityWebRequestTexture.GetTexture(url))
            {
                yield return www.SendWebRequest();

                if (www.result != UnityWebRequest.Result.Success)
                {
                    Debug.LogError($"Failed to load image {filename}: {www.error}");
                    yield break;
                }

                Texture2D texture = DownloadHandlerTexture.GetContent(www);
                if (texture != null)
                {
                    // Cache the texture
                    imageCache[filename] = texture;

                    // Display the image
                    imageDisplay.texture = texture;
                    currentImageIndex = index;

                    Debug.Log($"✅ Loaded and showing image {index + 1}/{imageFilenames.Count}: {filename} ({texture.width}x{texture.height})");
                }
                else
                {
                    Debug.LogError($"Failed to create texture from {filename}");
                }
            }
        }

        // Navigation methods equivalent to C++ implementation
        public void NextImage()
        {
            if (!imagesLoaded || isTransitioning || imageFilenames.Count == 0)
                return;

            int nextIndex = (currentImageIndex + 1) % imageFilenames.Count;
            StartCoroutine(LoadAndShowImage(nextIndex));
        }

        public void PreviousImage()
        {
            if (!imagesLoaded || isTransitioning || imageFilenames.Count == 0)
                return;

            int prevIndex = (currentImageIndex - 1 + imageFilenames.Count) % imageFilenames.Count;
            StartCoroutine(LoadAndShowImage(prevIndex));
        }

        public void ShowRandomImage()
        {
            if (!imagesLoaded || isTransitioning || imageFilenames.Count == 0)
                return;

            int randomIndex = UnityEngine.Random.Range(0, imageFilenames.Count);
            StartCoroutine(LoadAndShowImage(randomIndex));
        }

        // Input handling equivalent to C++ SDL2 input
        void Update()
        {
            if (!imagesLoaded)
                return;

            // Keyboard navigation (equivalent to C++ SDL2 key handling)
            if (Input.GetKeyDown(KeyCode.Space) || Input.GetKeyDown(KeyCode.RightArrow))
            {
                NextImage();
            }
            else if (Input.GetKeyDown(KeyCode.Backspace) || Input.GetKeyDown(KeyCode.LeftArrow))
            {
                PreviousImage();
            }
            else if (Input.GetKeyDown(KeyCode.R))
            {
                ShowRandomImage();
            }
            else if (Input.GetKeyDown(KeyCode.Escape))
            {
                Application.Quit();
            }
        }

        // Utility methods
        public int GetCurrentImageIndex() => currentImageIndex;
        public int GetImageCount() => imageFilenames.Count;
        public string GetCurrentImageName() =>
            imageFilenames.Count > 0 ? imageFilenames[currentImageIndex] : "None";

        // Cleanup cached textures when destroyed
        void OnDestroy()
        {
            foreach (var texture in imageCache.Values)
            {
                if (texture != null)
                {
                    Destroy(texture);
                }
            }
            imageCache.Clear();
        }
    }
}
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
