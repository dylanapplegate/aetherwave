# **Aetherwave**

Aetherwave is an immersive media engine for displaying cinematic MidJourney art across multiple monitors on macOS. Designed for personal storytelling, it emphasizes **stillness**, **mood**, and **fluidity**, without distraction.

## **🧠 Philosophy**

- Art is the hero—everything else disappears
- Motion and transitions should be **cinematic**, **intentional**, and **low-noise**
- Traditional, handcrafted programming preferred over AI guesswork

## **✨ Features**

- Multi-monitor image/video playback (fullscreen, split-screen, mosaic)
- Classification-based transitions (mood, tone, dominant color)
- Elegant transitions: fade, zoom, slide, drift
- Optional audio sync with bloom/beat detection
- Works with flat folder structures—no manual sorting required

## **🧱 Technologies**

- **C++ / openFrameworks**: Real-time rendering
- **Python**: Metadata extraction, classification
- **Electron (React/Svelte)**: Optional lightweight control panel
- **YAML / JSON**: Configuration and metadata

## **🚀 Getting Started**

### **1\. Setup openFrameworks (macOS, Apple Silicon)**

- Follow the openFrameworks installation guide
- Clone this repo into the `apps/myApps/` folder
- Use `projectGenerator` to create `.xcodeproj`

### **2\. Set up Python Environment**

cd src/python
python3 \-m venv venv
source venv/bin/activate
pip install \-r requirements.txt

### **3\. Run classification stub**

python classify.py /path/to/image/folder

Outputs to `config/metadata.json`

### **4\. Build the C++ App**

cd src/cpp
make run \# or open in Xcode and run

## **📁 Project Layout**

```text
./
├── assets
├── config
├── pbcopy
├── readme.md
├── src
│   ├── cpp
│   ├── python
│   ├── shared
│   └── ui
└── tests

```

## **🧼 Contribution Guidelines**

- See `github-instructions.md` for strict coding and contribution standards
- Use `black` and `pylint` (Python), `clang-format` (C++)
- Submit PRs with focused changes, descriptive titles, and test coverage

## **🔒 Privacy and Asset Handling**

- No cloud upload or syncing by default
- All assets remain local
- No personally identifying info should be embedded

---
