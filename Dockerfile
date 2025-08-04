# Multi-stage Docker build for Aetherwave
# Stage 1: Python environment with classification dependencies
FROM python:3.11-slim as python-base

WORKDIR /app

# Install system dependencies for OpenCV and image processing
RUN apt-get update && apt-get install -y \
    libopencv-dev \
    python3-opencv \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender-dev \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Copy Python requirements and install dependencies
COPY src/python/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Stage 2: Development environment with hot reload
FROM python-base as development

# Install development dependencies
RUN pip install --no-cache-dir pytest black pylint watchdog

# Copy source code
COPY src/python/ ./src/python/
COPY config/ ./config/
COPY tests/ ./tests/

# Expose port for FastAPI
EXPOSE 8000

# Default command for development
CMD ["python", "src/python/main.py"]

# Stage 3: Production environment
FROM python-base as production

# Copy only necessary files
COPY src/python/ ./src/python/
COPY config/ ./config/

# Create non-root user for security
RUN useradd -m -u 1000 aetherwave && chown -R aetherwave:aetherwave /app
USER aetherwave

EXPOSE 8000

# Production command
CMD ["python", "src/python/main.py"]
