# Copilot Instructions for Aetherwave

### **Overview for Autonomous Coding Agent**

You are contributing to the `Aetherwave` project—a media display engine for showcasing high-resolution MidJourney artwork across multiple monitors. This project is personal, cinematic, and minimalistic in nature. Your role is to **strictly scaffold and extend code within boundaries** defined in this instruction set.

### **✅ Authorized Tasks**

- Create or modify files in:
  - `src/`
  - `config/`
  - `tests/`
  - `.github/`
- Perform metadata analysis on placeholder assets only
- Use SDL2 (C++) for rendering and graphics with hardware acceleration
- Use Python for classification logic
- Output only compilable or lint-clean code

### **❌ Forbidden Actions**

- Do NOT embed credentials or hardcoded file paths
- Do NOT auto-fetch or download assets
- Do NOT touch `.DS_Store`, `.env`, system settings, or user folders
- Do NOT alter `assets/` (read-only unless instructed)
- Do NOT use black-box AI models for classification (use colorthief, OpenCV, etc.)

### **🧹 Project Cleanup & Verification Protocol**

**Before completing any autonomous coding session, ALWAYS perform these cleanup steps:**

1. **Remove Old Dependencies & Test Files:**

   ```bash
   # Remove any old library installations
   rm -rf libs/ external/ third_party/
   # Remove temporary test files
   find . -name "*_temp*" -delete
   find . -name "*_test*" -not -path "./tests/*" -delete
   find . -name "*.tmp" -delete
   # Remove old build artifacts
   rm -rf *.o *.obj *.exe *.dll *.dylib
   ```

2. **Legacy Test Cleanup:**

   ```bash
   # CRITICAL: Only remove tests that are truly legacy/obsolete
   # NEVER remove tests for active functionality
   #
   # Safe to remove:
   # - Tests expecting old API versions (e.g., 1.0.0 when current is 2.0.0)
   # - Tests for deprecated endpoints or schemas
   # - Tests for removed functionality
   #
   # NEVER remove:
   # - Tests for current API endpoints
   # - Tests for active core functionality
   # - Tests for business logic that's still in use
   #
   # Current active test files (DO NOT REMOVE):
   # - test_enhanced_api.py (API v2.0.0 tests)
   # - test_advanced_classification.py (core classification)
   # - test_content_theme_analyzer.py (theme analysis)
   #
   # Example legacy cleanup (only when identified as obsolete):
   # rm tests/python/test_metadata.py  # API v1.0.0 legacy tests
   ```

3. **Terminate All Background Processes:**

   ```bash
   # TERMINAL TIMING: Allow 10+ seconds for new terminal initialization
   # oh-my-zsh requires startup time before command execution

   # Kill any running development servers
   pkill -f "python.*main.py"
   pkill -f "fastapi"
   pkill -f "uvicorn"
   # Kill any running C++ applications
   pkill -f "Aetherwave"
   pkill -f "visual_sdl"
   # Clean up docker processes if used
   docker-compose down --remove-orphans
   ```

4. **Clean Terminal State:**

   ```bash
   # TERMINAL TIMING: Ensure terminal is fully loaded before checking
   # Wait for oh-my-zsh prompt to appear before executing commands

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

   # Must pass: C++ application build verification
   ./scripts/cpp-setup.sh
   ./scripts/verify-app.sh

   # CRITICAL: Ensure app processes are properly terminated
   # The verify-app.sh script should handle this automatically
   # If manual testing, always exit apps with ESC/Q keys or kill processes:
   pkill -f "Aetherwave"

   # Must pass: All tests
   pytest tests/ --tb=short
   # Test coverage goal: 90%+ for all Python and C++ modules
   # Use: pytest --cov=src --cov-report=term-missing
   # If coverage drops below 90%, add or update tests before completing session
   ```

6. **Final State Verification:**
   - ✅ No running background processes (`ps aux | grep -E "python.*main|Aetherwave|fastapi"` returns empty)
   - ✅ No hanging terminal jobs (`jobs -l` returns empty)
   - ✅ Python API builds and responds to health checks
   - ✅ C++ application builds without errors and launches successfully
   - ✅ C++ application can be started, tested, and cleanly terminated
   - ✅ All tests pass with 90%+ coverage
   - ✅ No temporary files or old dependencies remain

**CRITICAL**: An autonomous agent MUST NOT consider their work complete until ALL verification steps pass. If any step fails, troubleshoot and fix before ending the session.

**APP LIFECYCLE MANAGEMENT**:

- Visual applications must be properly started, verified to work, and cleanly terminated
- Never leave graphical applications running indefinitely
- Always verify applications can be exited gracefully (ESC/Q keys)
- Use `pkill -f "Aetherwave"` if processes don't terminate properly

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

   - Debug logger automatically clears log on each app startup (`std::ios::trunc`)
   - Comprehensive logging includes: image switching, layout calculations, window events, rendering details
   - Log file location: `build/bin/aetherwave_debug.log`

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

4. **What to Log and Track:**

   - Image switching operations with index changes (`DEBUG_LOG("IMAGE_SWITCH", ...)`)
   - Layout calculations with window/texture dimensions (`DEBUG_LOG("LAYOUT", ...)`)
   - Rendering details with aspect ratios (`DEBUG_LOG("RENDER", ...)`)
   - Window events with position/size changes (`DEBUG_WINDOW(...)`)
   - Display detection and DPI scaling (`DEBUG_DISPLAY(...)`)

5. **User Reporting Guidelines:**

   - Note specific visual issues: proportions wrong, sluggish controls, layout problems, quality issues
   - Specify which actions trigger problems
   - Always request debug log analysis before declaring issues fixed
   - Use phrase "Check the debug log" to trigger analysis

6. **Debug Log Analysis Focus:**
   - Look for inconsistent index updates in image switching
   - Check for excessive or missing layout calculations
   - Verify correct window/display detection
   - Identify aspect ratio calculation problems
   - Track texture dimension vs. destination rectangle mismatches

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
- For reading files, use:
  - `read_text_file` to get the full contents of a text file (optionally head/tail lines).
  - `read_media_file` to stream image/audio files as base64 with MIME type.
  - `read_multiple_files` to read several files at once (failed reads do not halt operation).
- For writing or editing files, use:
  - `write_file` to create or overwrite a file with new content.
  - `edit_file` for selective edits using pattern matching, indentation preservation, and dry-run previews.
- For directory management, use:
  - `create_directory` to create or ensure a directory exists.
  - `list_directory` to list contents with [FILE]/[DIR] prefixes.
- For moving/renaming, use:
  - `move_file` to move or rename files/directories (fails if destination exists).
- For searching and metadata:
  - `search_files` to recursively find files/directories by pattern, with exclusions.
  - `get_file_info` for detailed metadata (size, timestamps, type, permissions).
  - `list_allowed_directories` to see which directories are permitted for access.
- Always treat files as UTF-8 text unless using `read_media_file`.
- Preview edits with `edit_file` dryRun before applying changes.
- Use these commands for any file or directory access, modification, or inspection.

### **🧠 Coding Behavior Rules**

#### **C++**

- Use modern `C++17`
- All logic should be wrapped in `namespace Aetherwave`
- Split rendering logic and UI logic
- Use SDL2 for cross-platform graphics and window management
- Frame timing and sync via SDL2's VSync and timing functions
- No magic numbers—use constants and config files

#### **Python**

- Fully typed using `typing`
- Include docstrings on every function
- API endpoints must return JSON with predictable structure
- Classification stubs should print sample metadata and write JSON only to `config/`
- Use `pytest` and maintain 90%+ test coverage

#### **General**

- Maintain max 80-char line width where possible
- Include summary comment blocks on each class
- Create PRs per feature. Tag with scope: `feature/display-mode-fade`, `bugfix/color-metadata`
- All `README.md` and `config.yaml` updates must be declared in PR notes

## Project Overview

Aetherwave is a content-driven media display engine for showcasing high-resolution MidJourney artwork across multiple monitors. The project evolved from hardcoded cyberfemme aesthetics to intelligent content analysis that automatically generates appropriate visual themes.

**Key Business Decision (August 3, 2025)**: Content-driven aesthetic adaptation enables universal reusability while preserving artistic vision. Cyberfemme artwork naturally drives cyberfemme interfaces, but the same system adapts to any art collection.

## Architecture & Structure

### **Core Components:**

- **Python Classification Engine** (`src/python/`): ColorThief + OpenCV analysis, FastAPI endpoints
- **C++ Display Engine** (`src/cpp/`): SDL2 rendering with 60 FPS multi-monitor support
- **Theme Management** (`src/shared/`): Content-to-theme mapping with statistical confidence
- **Configuration System** (`config/`): YAML-based settings with theme caching

### **Content-Driven Architecture:**

```
Image Collection → Classification → Theme Detection → Visual Rendering
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
2. **Implementation**: Follow C++17/Python typing standards with comprehensive testing
3. **Documentation**: Update architecture docs and README files during development
4. **Memory Recording**: Store business decisions in MCP memory for conversational context
5. **Build Verification**: Ensure both Python API and C++ application build successfully
6. **Final Cleanup**: Execute project cleanup protocol before session completion
7. **PR Creation**: Tag with scope (`feature/`, `bugfix/`) and document config changes

### **Mandatory Verification Steps:**

**For ANY code changes, autonomous agents must verify:**

**Terminal Startup Considerations:**

- Allow 10+ seconds for terminal initialization when starting new terminal sessions
- oh-my-zsh and similar shell configurations require startup time before command execution
- Wait for prompt to appear before executing commands in new terminals

1. **Python API Verification:**

   ```bash
   # Start Docker services
   ./scripts/dev-setup.sh
   # Verify API health
   curl -s http://localhost:8000/health
   # Expected: {"healthy":true,"service_version":"2.0.0",...}
   ```

2. **C++ Application Verification:**

   ```bash
   # Build application
   ./scripts/cpp-setup.sh
   # Run comprehensive verification
   ./scripts/verify-app.sh
   # Expected: All ✅ checks pass, visual window appears
   ```

3. **Unity Build Verification (MANDATORY):**

   ```bash
   # CRITICAL: Must pass before claiming Unity setup success
   ./scripts/unity-build-verify.sh
   # Expected: "🎉 Unity Build Verification PASSED"
   # NOTE: URP shader warnings are normal and do not indicate compilation failure
   ```

4. **Test Suite Verification:**

   ```bash
   # Run all tests
   pytest tests/ --cov=src --cov-report=term-missing
   # Expected: 90%+ coverage, all tests pass
   ```

5. **Clean Shutdown:**
   ```bash
   # Stop all services
   ./scripts/dev-stop.sh
   pkill -f "Aetherwave"
   # Verify no processes remain
   ps aux | grep -E "python.*main|Aetherwave|fastapi"
   ```

### **Build Commands:**

**Docker Workflow (Recommended):**

- **One-Command Setup**: `./scripts/dev-setup.sh` (includes Docker build and service startup)
- **Stop Services**: `./scripts/dev-stop.sh`
- **Rebuild After Changes**: `docker-compose build && docker-compose up -d`
- **View Logs**: `docker-compose logs -f classification-api`

**C++ Display Engine:**

- **One-Command Setup**: `./scripts/cpp-setup.sh` (includes SDL2 installation and build)
- **Run Application**: `./scripts/cpp-run.sh`
- **Verify Installation**: `./scripts/verify-app.sh`
- **Clean Rebuild**: `./scripts/cpp-setup.sh clean`

**Unity Development:**

- **Unity Setup**: `./scripts/unity-setup.sh` (creates Unity project structure and dependencies)
- **Unity Build Verification**: `./scripts/unity-build-verify.sh` (**MANDATORY** before claiming Unity setup success)
- **Unity Linting**: `./scripts/unity-lint.sh` (C# code quality validation)
- **Unity Requirements**: Unity 6.1+ (6000.1.14f1), Universal Render Pipeline (URP), C# scripting

**Manual Workflow:**

- **Python Setup**: `python -m venv venv && source venv/bin/activate && pip install -r requirements.txt`
- **C++ Build**: `mkdir build && cd build && cmake .. && make`
- **Unity Build**: `./scripts/unity-build-verify.sh` (**REQUIRED** - must pass before claiming success)
- **Testing**: `pytest tests/` (90%+ coverage requirement)
- **Classification**: `python src/python/main.py` (FastAPI server on port 8000)

## Key Conventions

### **Naming & Structure:**

- All C++ logic wrapped in `namespace Aetherwave`
- Python functions fully typed with comprehensive docstrings
- Configuration via YAML files, no hardcoded values
- 80-character line width where possible

### **Theme System Convention:**

- **Content Analysis** → **Statistical Theme Detection** → **Dynamic Visual Application**
- Themes determined by color dominance, mood analysis, and confidence scoring
- Universal adaptability without manual configuration required

## Critical Files & Directories

### **Core Implementation:**

- `src/python/content_theme_analyzer.py`: Collection analysis and theme detection algorithms
- `src/python/main.py`: FastAPI server with `/analyze/collection-theme` endpoint
- `src/cpp/visual_sdl.cpp`: SDL2-based visual display engine with real-time rendering
- `src/shared/Config.cpp`: Configuration management and asset handling
- `config/themes.yaml`: Theme configuration and caching system

### **Documentation & Planning:**

- `/docs/FEATURE_ROADMAP.md`: Business decisions, implementation status, future planning
- `.github/copilot-instructions.md`: Development guidelines and architectural decisions
- `README.md`: Current setup instructions and project overview

## Dependencies & Integration Points

### **Python Dependencies:**

- **FastAPI 2.0**: API server with theme analysis endpoints
- **ColorThief**: Dominant color extraction from images
- **OpenCV**: Advanced computer vision and image analysis
- **Pytest**: Testing framework with 90%+ coverage requirement

### **C++ Dependencies:**

- **SDL2**: Cross-platform graphics and window management
- **SDL2_image**: Image loading and texture creation
- **Modern C++17**: Performance-optimized display engine

### **External Integrations:**

- **MidJourney Assets**: Read-only access to high-resolution artwork
- **Multi-Monitor Setup**: macOS display coordination
- **Theme Caching**: File-based caching for collection analysis

## Testing Strategy

### **Comprehensive Test Coverage:**

- **Unit Tests**: Individual component functionality (90%+ coverage)
- **Integration Tests**: End-to-end content→theme→visual pipeline validation
- **Performance Tests**: 60 FPS rendering with large collections
- **Theme Accuracy Tests**: Statistical validation of content-to-theme mapping

#### **Test Coverage Goals**

- All Python and C++ modules must maintain **90%+ test coverage**
- Use `pytest --cov=src --cov-report=term-missing` for Python coverage
- Use C++ coverage tools (e.g., gcov, lcov) for C++ modules
- If coverage drops below 90%, add or update tests before completing any coding session

### **Quality Assurance:**

- **Linting**: Python typing validation, C++ modern standards compliance
- **Business Logic Testing**: Theme detection accuracy validation
- **Visual Regression**: Screenshot-based UI consistency testing

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
- **Local Dependencies**: Direct installation of OpenCV, FastAPI, etc.
- **Native C++ Build**: Direct openFrameworks compilation

### **Production Considerations:**

- **4K+ Display Support**: Optimized for high-resolution multi-monitor setups
- **Performance Requirements**: 60 FPS sustained rendering with visual effects
- **Collection Scalability**: Efficient analysis of large image collections

---

**Note**: This file should be updated as the codebase grows to include:

- Specific architectural patterns and component relationships
- Build/test commands that aren't obvious from package.json
- Project-specific conventions that differ from standard practices
- Integration points and external service dependencies
- Key files that exemplify important patterns
