# Copilot Instructions for Aetherwave

### **Overview for Autonomous Coding Agent**

You are contributing to the `Aetherwave` project—a media display engine for showcasing high-resolution MidJourney artwork across multiple monitors. This project uses a **Qt + Python architecture** for professional gallery installations with content-driven aesthetic adaptation.

### **✅ Authorized Tasks**

- Create or modify files in:
  - `src/python/` (FastAPI backend and Qt frontend)
  - `config/`
  - `tests/`
  - `.github/`
- Perform metadata analysis on placeholder assets only
- Use Qt + PySide6 for professional gallery displays
- Use Python for classification logic and UI development
- Output only lint-clean code

### **❌ Forbidden Actions**

- Do NOT embed credentials or hardcoded file paths
- Do NOT auto-fetch or download assets
- Do NOT touch `.DS_Store`, `.env`, system settings, or user folders
- Do NOT alter `assets/` (read-only unless instructed)
- Do NOT use black-box AI models for classification (use colorthief, OpenCV, etc.)

### **🔍 Feature Verification Protocol**

**Before marking any feature as "COMPLETED" in documentation, ALWAYS verify functionality:**

1. **Phase 4A Content-Driven Aesthetics Verification:**

   ```bash
   # MANDATORY: Test Python classification API
   source venv/bin/activate
   python -m uvicorn src.python.main:app --host 0.0.0.0 --port 8000 --reload &
   sleep 3

   # Test individual image classification (should complete <1 second)
   curl -X POST "http://localhost:8000/classify" \
     -H "Content-Type: application/json" \
     -d '{"image_path": "assets/images/06201422-D51B-414B-89DA-E12CACAB28CE.png", "include_metadata": true}' \
     | jq '.metadata.dominant_color'

   # Test collection theme analysis (should complete <0.1 seconds)
   curl -X POST "http://localhost:8000/analyze/collection-theme" \
     -H "Content-Type: application/json" \
     -d '{"collection_path": "assets/images", "sample_size": 5}' \
     | jq '.theme.theme_name'

   # EXPECTED: API responses with valid color/theme data, not error messages
   pkill -f "uvicorn"
   ```

2. **Qt Gallery Application Verification:**

   ```bash
   # MANDATORY: Test Qt application launches and connects to API
   source venv/bin/activate
   python -m uvicorn src.python.main:app --host 0.0.0.0 --port 8000 --reload &
   sleep 3
   
   # Test Qt application startup
   ./scripts/fe-run.sh
   # EXPECTED: Gallery window opens, connects to API, displays images   pkill -f "uvicorn"
   pkill -f "python.*main.py"
   ```

3. **Performance Benchmarks:**

   ```bash
   # Classification performance must meet standards:
   # - Individual images (1-7MB PNG): <1.0 seconds
   # - Collection analysis (5 images): <0.1 seconds
   # - ColorThief quality=10 with image preprocessing for large files
   ```

4. **Integration Testing:**

   ```bash
   # MANDATORY: Test complete content→theme→visual pipeline
   # 1. Python API generates theme from image collection
   # 2. Qt app receives theme data via HTTP
   # 3. Visual rendering adapts to detected theme
   # EXPECTED: End-to-end workflow functional
   ```

**CRITICAL**: Never mark features as "COMPLETED" without passing ALL verification steps. If verification fails, fix the issues or update documentation to reflect actual status.

### **🧹 Project Cleanup & Verification Protocol**

**Before completing any autonomous coding session, ALWAYS perform these cleanup steps:**

1. **Remove Old Dependencies & Test Files:**

   ```bash
   # Remove temporary test files
   find . -name "*_temp*" -delete
   find . -name "*_test*" -not -path "./tests/*" -delete
   find . -name "*.tmp" -delete
   ```

2. **Legacy Test Cleanup:**

   ```bash
   # CRITICAL: Only remove tests that are truly legacy/obsolete
   # Current active test files (DO NOT REMOVE):
   # - test_enhanced_api.py (API v2.0.0 tests)
   # - test_advanced_classification.py (core classification)
   # - test_content_theme_analyzer.py (theme analysis)
   ```

3. **Terminate All Background Processes:**

   ```bash
   # Kill any running development servers
   pkill -f "python.*main.py"
   pkill -f "fastapi"
   pkill -f "uvicorn"
   ```

4. **Clean Terminal State:**

   ```bash
   # Ensure no hanging background jobs
   jobs -l
   # Kill any background jobs if present
   kill %1 %2 %3 2>/dev/null || true
   ```

5. **Verify Complete Build Pipeline:**

   ```bash
   # Must pass: Python API build verification
   ./scripts/dev-setup.sh
   curl -s http://localhost:8000/health | grep -q "healthy"

   # Must pass: Qt application verification
   ./scripts/fe-run.sh
   # Expected: Gallery window opens successfully

   # Must pass: All tests
   pytest tests/ --tb=short
   # Test coverage goal: 90%+ for all Python modules
   ```

6. **Final State Verification:**
   - ✅ No running background processes (`ps aux | grep -E "python.*main|fastapi"` returns empty)
   - ✅ No hanging terminal jobs (`jobs -l` returns empty)
   - ✅ Python API builds and responds to health checks
   - ✅ Qt application launches successfully
   - ✅ All tests pass with 90%+ coverage
   - ✅ No temporary files remain

**CRITICAL**: An autonomous agent MUST NOT consider their work complete until ALL verification steps pass. If any step fails, troubleshoot and fix before ending the session.

### **🧠 Memory & Documentation Management**

**Follow these steps for each interaction:**

1. **User Identification:**
   - Assume you are interacting with `default_user`.
   - If you have not identified `default_user`, proactively try to do so.
2. **Memory Retrieval:**
   - Always begin your chat by saying only "Remembering..." and retrieve all relevant business and technical information from your knowledge graph.
   - Always refer to your knowledge graph as your "memory".
3. **Memory Gathering:**
   - While conversing with the user, be attentive to any new information that falls into these categories:
     - **Business Decisions:** Key decisions made, the reasoning behind them, and any trade-offs or alternative solutions considered.
     - **Technical Information:** Design patterns, architectural decisions, and specific solutions that were tried and found not to work.
     - **Code-related Changes:** Any significant modifications to core business logic or API endpoints.
     - **Project Context:** Information about clients, project goals, and stakeholder feedback.
4. **Memory Update:**
   - If any new information was gathered during the interaction, update your memory as follows:
     - Create entities for recurring organizations, people, and significant project-related items.
     - Connect them to the current entities using relations (e.g., `decided_on`, `is_related_to`, `discovered_does_not_work`).
     - Store facts about them as observations, including the `timestamp` and, if possible, the `decision_maker`.

### **🧪 Interactive Debugging Protocol**

**For complex visual and multi-monitor issues, use this collaborative debugging approach:**

1. **Setup Fresh Debug Logging:**

   - Debug logger automatically clears log on each app startup
   - Comprehensive logging includes: image switching, layout calculations, window events, rendering details
   - Log file location: `logs/aetherwave_debug.log`

2. **Debug Session Protocol:**

   ```
   AGENT: "Please start the application and perform these tests in order:"
   USER: [Performs testing sequence]
   USER: "Check the debug log"
   AGENT: [Analyzes debug log for root cause]
   ```

3. **Standard Test Sequence:**

   - Start application and wait for initial load
   - Press SPACE/→ 3-4 times (test next image functionality)
   - Press BACKSPACE/← 2-3 times (test previous image functionality)
   - Move window to laptop monitor (test multi-monitor positioning)
   - Try next/previous on laptop monitor (test consistency)
   - Resize window (test layout recalculation)
   - Move back to main monitor (test return positioning)

**CRITICAL**: Never declare issues fixed without collaborative debug log analysis and user confirmation.

### **📚 Documentation Maintenance Protocol**

**Maintain living documentation that evolves with the project:**

1. **Business Decision Documentation:**

   - **ALWAYS** update `/docs/FEATURE_ROADMAP.md` when business decisions are made
   - Record the decision, reasoning, alternatives considered, and implementation timeline
   - Include confidence levels and success metrics for major architectural choices
   - Document trade-offs and potential future reconsiderations

2. **Technical Architecture Visibility:**

   - **ALWAYS** update this `copilot-instructions.md` file when:
     - New architectural patterns emerge
     - Key dependencies are added or changed
     - Development workflow changes
     - Testing strategies evolve
   - Replace placeholder sections as the codebase develops
   - Maintain current examples of key patterns and conventions

3. **Application Functionality Documentation:**

   - **ALWAYS** maintain up-to-date `README.md` files in each major directory (`src/`, `config/`, `tests/`)
   - Document how components work together, not just what they do
   - Include usage examples and integration patterns
   - Update API documentation when endpoints change

4. **Decision Audit Trail:**

   - **DUAL STORAGE**: Store all business decisions in both MCP memory AND documentation files
   - MCP memory for conversational context and relationship tracking
   - Documentation files for team onboarding and historical reference
   - Cross-reference between memory entities and documentation sections

5. **Documentation Quality Standards:**
   - **Accuracy**: Documentation must reflect current implementation, not future plans
   - **Completeness**: Every major component should have purpose, usage, and integration docs
   - **Accessibility**: Technical decisions should be understandable to future contributors
   - **Maintenance**: Documentation updates are part of every significant code change

### MCP Server Tools

- Please use Context7 to get the latest library documentation.
- Use Sequential Thinking to break down complex tasks into manageable steps.
- Use Memory to keep track of business decisions, technical information, and project context.
- Use Deepwiki to access detailed documentation and examples for specific libraries or frameworks. Use as backup to Context7.
- Use GitHub MCP for all agentic workflow automation, including branch creation, PR management, and workflow status checks.
- Always use the filesystem MCP server for all file and directory operations.

### **🧠 Coding Behavior Rules**

#### **Python**

- Fully typed using `typing`
- Include docstrings on every function
- API endpoints must return JSON with predictable structure
- Qt applications should follow Model-View-Controller patterns
- Classification stubs should print sample metadata and write JSON only to `config/`
- Use `pytest` and maintain 90%+ test coverage

#### **Qt + PySide6**

- Use Qt Designer (.ui files) for complex layouts when appropriate
- Follow Qt signals and slots patterns for event handling
- Implement proper resource management (close events, cleanup)
- Use QThread for background operations to keep UI responsive
- Support high-DPI displays and multi-monitor setups

#### **General**

- Maintain max 80-char line width where possible
- Include summary comment blocks on each class
- Create PRs per feature. Tag with scope: `feature/display-mode-fade`, `bugfix/color-metadata`
- All `README.md` and `config.yaml` updates must be declared in PR notes

## Project Overview

Aetherwave is a content-driven media display engine for showcasing high-resolution MidJourney artwork across multiple monitors. The project uses **Qt + Python architecture** for professional gallery installations with intelligent content analysis that automatically generates appropriate visual themes.

**Key Business Decision (January 2025)**: Qt + Python migration eliminates Unity/C++ complexity while preserving all backend investments. Single-language stack accelerates development velocity and reduces maintenance overhead.

## Architecture & Structure

### **Core Components:**

- **Python Classification Engine** (`src/python/api/`): ColorThief + OpenCV analysis, FastAPI endpoints
- **Qt Gallery Frontend** (`src/python/qt_app/`): Professional gallery display with multi-monitor support
- **Theme Management** (`src/python/shared/`): Content-to-theme mapping with statistical confidence
- **Configuration System** (`config/`): YAML-based settings with theme caching

### **Content-Driven Architecture:**

```
Image Collection → Classification → Theme Detection → Qt Rendering
     ↓               ↓                ↓               ↓
  Metadata      Color/Mood       Aesthetic        Dynamic UI
  Analysis      Extraction       Selection        Generation
```

## Development Workflow

### Git and branching

1. **Branch Naming**: Use `feature/`, `bugfix/`, or `hotfix/` prefixes
2. **Branch Creation**: Use GitHub MCP to create branches for new features or fixes.
3. **Summary of Work Done**: At the end of the session, summarize the work done for me to check and verify. Let this be a checklist for me to go through and verify that everything is working as expected. I'll review the changes and ensure they meet expectations. Once I give the okay, you can proceed with the PR creation.
4. **PR Description**: When creating a PR, provide a clear description of the changes made, including any relevant context or links to related issues.

### **Standard Development Process:**

1. **Feature Planning**: Update `/docs/FEATURE_ROADMAP.md` with implementation timeline
2. **Implementation**: Follow Python typing standards with comprehensive testing
3. **Documentation**: Update architecture docs and README files during development
4. **Memory Recording**: Store business decisions in MCP memory for conversational context
5. **Build Verification**: Ensure both Python API and Qt application work successfully
6. **Final Cleanup**: Execute project cleanup protocol before session completion
7. **PR Creation**: Tag with scope (`feature/`, `bugfix/`) and document config changes

### **Mandatory Verification Steps:**

**For ANY code changes, autonomous agents must verify:**

1. **Python API Verification:**

   ```bash
   # Start API services
   ./scripts/dev-setup.sh
   # Verify API health
   curl -s http://localhost:8000/health
   # Expected: {"healthy":true,"service_version":"2.0.0",...}
   ```

2. **Qt Application Verification:**

   ```bash
   # Start Qt application
   ./scripts/qt-run.sh
   # Expected: Gallery window opens, connects to API
   ```

3. **Test Suite Verification:**

   ```bash
   # Run all tests
   pytest tests/ --cov=src --cov-report=term-missing
   # Expected: 90%+ coverage, all tests pass
   ```

4. **Clean Shutdown:**
   ```bash
   # Stop all services
   ./scripts/dev-stop.sh
   pkill -f "python.*main"
   # Verify no processes remain
   ps aux | grep -E "python.*main|fastapi"
   ```

### **Build Commands:**

**Docker Workflow (Recommended):**

- **One-Command Setup**: `./scripts/dev-setup.sh` (includes Docker build and service startup)
- **Stop Services**: `./scripts/dev-stop.sh`
- **Rebuild After Changes**: `docker-compose build && docker-compose up -d`
- **View Logs**: `docker-compose logs -f classification-api`

**Qt Application:**

- **Qt Setup**: `./scripts/fe-run.sh` (starts Qt gallery application)
- **Manual Setup**: `python src/python/main.py` (direct application launch)

**Manual Workflow:**

- **Python Setup**: `python -m venv venv && source venv/bin/activate && pip install -r requirements.txt`
- **Testing**: `pytest tests/` (90%+ coverage requirement)
- **Classification**: `python -m uvicorn src.python.api.main:app --reload` (FastAPI server on port 8000)
- **Gallery**: `python src/python/main.py` (Qt application)

## Key Conventions

### **Naming & Structure:**

- All Python modules follow PEP 8 naming conventions
- Qt classes use CamelCase following Qt conventions
- Configuration via YAML files, no hardcoded values
- 80-character line width where possible

### **Theme System Convention:**

- **Content Analysis** → **Statistical Theme Detection** → **Dynamic Visual Application**
- Themes determined by color dominance, mood analysis, and confidence scoring
- Universal adaptability without manual configuration required

## Critical Files & Directories

### **Core Implementation:**

- `src/python/api/main.py`: FastAPI server with theme analysis endpoints
- `src/python/qt_app/gallery_window.py`: Qt gallery display with multi-monitor support
- `src/python/qt_app/api_client.py`: HTTP client for backend integration
- `src/python/shared/content_theme_analyzer.py`: Collection analysis and theme detection
- `config/themes.yaml`: Theme configuration and caching system

### **Documentation & Planning:**

- `/docs/FEATURE_ROADMAP.md`: Business decisions, implementation status, future planning
- `.github/copilot-instructions.md`: Development guidelines and architectural decisions
- `README.md`: Current setup instructions and project overview

## Dependencies & Integration Points

### **Python Dependencies:**

- **FastAPI 2.0**: API server with theme analysis endpoints
- **PySide6**: Professional Qt-based gallery frontend
- **ColorThief**: Dominant color extraction from images
- **OpenCV**: Advanced computer vision and image analysis
- **Pytest**: Testing framework with 90%+ coverage requirement

### **External Integrations:**

- **MidJourney Assets**: Read-only access to high-resolution artwork
- **Multi-Monitor Setup**: macOS display coordination
- **Theme Caching**: File-based caching for collection analysis

## Testing Strategy

### **Comprehensive Test Coverage:**

- **Unit Tests**: Individual component functionality (90%+ coverage)
- **Integration Tests**: End-to-end content→theme→visual pipeline validation
- **Performance Tests**: Responsive Qt UI with large collections
- **Theme Accuracy Tests**: Statistical validation of content-to-theme mapping

#### **Test Coverage Goals**

- All Python modules must maintain **90%+ test coverage**
- Use `pytest --cov=src --cov-report=term-missing` for Python coverage
- If coverage drops below 90%, add or update tests before completing any coding session

### **Quality Assurance:**

- **Linting**: Python typing validation, Qt best practices
- **Business Logic Testing**: Theme detection accuracy validation
- **Visual Regression**: Qt widget and layout consistency testing

## Deployment & Environment

### **Development Environment:**

- **macOS**: Primary development and testing platform
- **Docker**: Containerized development environment for consistent setup
- **Multi-Monitor Setup**: Testing environment for gallery installations
- **Local Asset Management**: Placeholder assets for development testing

### **Development Workflow:**

**Docker-First Approach (Recommended):**

- **Setup**: `./scripts/dev-setup.sh` creates entire development environment
- **Development**: Hot reload enabled, volume mounts for live code editing
- **Testing**: Run tests inside containers or locally with pytest
- **Deployment**: Multi-stage builds support both development and production

**Manual Setup (Alternative):**

- **Python Virtual Environment**: For developers needing custom configurations
- **Local Dependencies**: Direct installation of PySide6, FastAPI, etc.
- **Native Qt Build**: Direct PySide6 installation and compilation

### **Production Considerations:**

- **4K+ Display Support**: Optimized for high-resolution multi-monitor setups
- **Performance Requirements**: 60 FPS sustained rendering with Qt graphics
- **Collection Scalability**: Efficient analysis of large image collections

---

**Note**: This file should be updated as the codebase grows to include:

- Specific Qt patterns and component relationships
- Build/test commands that aren't obvious from scripts
- Project-specific conventions that differ from standard practices
- Integration points and external service dependencies
- Key files that exemplify important patterns