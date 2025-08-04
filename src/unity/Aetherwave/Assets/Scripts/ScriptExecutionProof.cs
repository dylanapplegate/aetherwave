using UnityEngine;
using System.IO;

namespace Aetherwave
{
    /// <summary>
    /// Simple script to prove our code is running by writing to a file
    /// </summary>
    public static class ScriptExecutionProof
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        static void ProveExecution()
        {
            string timestamp = System.DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            string message = $"🎯 PROOF: Our C# scripts ARE running at {timestamp}\n";

            Debug.Log(message);

            // Write to a file we can check
            try
            {
                string filePath = Path.Combine(Application.persistentDataPath, "script_execution_proof.txt");
                File.AppendAllText(filePath, message);
                Debug.Log($"🎯 Wrote proof to: {filePath}");
            }
            catch (System.Exception e)
            {
                Debug.LogError($"🎯 Failed to write proof file: {e.Message}");
            }

            // Also force all cameras to bright cyan to make it visually obvious
            Camera[] cameras = Object.FindObjectsByType<Camera>(FindObjectsSortMode.None);
            foreach (Camera cam in cameras)
            {
                cam.clearFlags = CameraClearFlags.SolidColor;
                cam.backgroundColor = Color.cyan;
                Debug.Log($"🎯 Set camera {cam.name} to CYAN");
            }

            Debug.Log($"🎯 ScriptExecutionProof completed - set {cameras.Length} cameras to CYAN");
        }
    }
}
