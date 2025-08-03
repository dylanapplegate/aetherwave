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
- Use only openFrameworks (C++) for rendering and audio
- Use Python for classification logic
- Output only compilable or lint-clean code

### **❌ Forbidden Actions**

- Do NOT embed credentials or hardcoded file paths
- Do NOT auto-fetch or download assets
- Do NOT touch `.DS_Store`, `.env`, system settings, or user folders
- Do NOT alter `assets/` (read-only unless instructed)
- Do NOT use black-box AI models for classification (use colorthief, OpenCV, etc.)

### **🧠 Memory Management**

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

### MCP Server Tools

- Please use Context7 to get the latest library documentation.
- Use Sequential Thinking to break down complex tasks into manageable steps.
- Use Memory to keep track of business decisions, technical information, and project context.
- Use Deepwiki to access detailed documentation and examples for specific libraries or frameworks. Use as backup to Context7.

### **🧠 Coding Behavior Rules**

#### **C++**

- Use modern `C++17`Z
- All logic should be wrapped in `namespace Aetherwave`
- Split rendering logic and UI logic
- Frame timing and sync via `ofGetElapsedTimef()` or similar tools
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

Aetherwave is an early-stage project. These instructions will be updated as the codebase develops.

## Architecture & Structure

_To be documented as the project structure emerges_

## Development Workflow

_To be documented once build tools and processes are established_

## Key Conventions

_To be documented as coding patterns emerge_

## Critical Files & Directories

_To be documented as the project structure develops_

## Dependencies & Integration Points

_To be documented as external dependencies are added_

## Testing Strategy

_To be documented once testing framework is chosen_

## Deployment & Environment

_To be documented once deployment strategy is established_

---

**Note**: This file should be updated as the codebase grows to include:

- Specific architectural patterns and component relationships
- Build/test commands that aren't obvious from package.json
- Project-specific conventions that differ from standard practices
- Integration points and external service dependencies
- Key files that exemplify important patterns
