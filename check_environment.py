#!/usr/bin/env python3
"""
VS Code Environment Verification Script

This script helps verify that VS Code can properly resolve imports
and access the virtual environment dependencies.
"""

def check_imports():
    """Check that all required imports can be resolved."""
    print("🔍 Checking Python environment...")
    
    try:
        import sys
        print(f"✅ Python executable: {sys.executable}")
        print(f"✅ Python version: {sys.version}")
        
        import fastapi
        print(f"✅ FastAPI: {fastapi.__version__}")
        
        import pydantic
        print(f"✅ Pydantic: {pydantic.__version__}")
        
        import uvicorn
        print("✅ Uvicorn available")
        
        import aiofiles
        print("✅ Aiofiles available")
        
        import pytest
        print(f"✅ Pytest: {pytest.__version__}")
        
    except ImportError as e:
        print(f"❌ Import error: {e}")
        return False
    
    print("\n🎉 All imports resolved successfully!")
    return True

if __name__ == "__main__":
    check_imports()
