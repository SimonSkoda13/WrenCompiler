#!/bin/bash
# Helper script for Docker development workflow

set -e

CMD=${1:-help}

case "$CMD" in
    start)
        echo "🚀 Starting development container..."
        docker-compose up -d dev
        docker-compose exec dev /bin/bash
        ;;
    
    stop)
        echo "🛑 Stopping containers..."
        docker-compose down
        ;;
    
    build)
        echo "🔨 Building Docker images..."
        docker-compose build
        ;;
    
    rebuild)
        echo "🔨 Rebuilding Docker images (no cache)..."
        docker-compose build --no-cache
        ;;
    
    compile)
        echo "⚙️  Compiling project..."
        docker-compose run --rm dev make clean all
        ;;
    
    test)
        echo "🧪 Running tests..."
        docker-compose run --rm dev make test
        ;;
    
    valgrind)
        echo "🔍 Running valgrind..."
        docker-compose run --rm dev make valgrind
        ;;
    
    interpreter)
        echo "🎯 Starting interpreter container..."
        docker-compose run --rm interpreter /bin/bash
        ;;
    
    shell)
        echo "💻 Opening shell in dev container..."
        docker-compose run --rm dev /bin/bash
        ;;
    
    format)
        echo "✨ Formatting code..."
        docker-compose run --rm dev make format
        ;;
    
    lint)
        echo "🔎 Running static analysis..."
        docker-compose run --rm dev make lint
        ;;
    
    clean)
        echo "🧹 Cleaning build artifacts..."
        docker-compose run --rm dev make clean
        ;;
    
    logs)
        echo "📋 Showing container logs..."
        docker-compose logs -f
        ;;
    
    help|*)
        echo "IFJ Compiler Development Helper"
        echo ""
        echo "Usage: ./dev.sh <command>"
        echo ""
        echo "Commands:"
        echo "  start       - Start development container with interactive shell"
        echo "  stop        - Stop all containers"
        echo "  build       - Build Docker images"
        echo "  rebuild     - Rebuild Docker images from scratch"
        echo "  compile     - Compile the project"
        echo "  test        - Run tests"
        echo "  valgrind    - Run with valgrind memory checker"
        echo "  interpreter - Start interpreter container"
        echo "  shell       - Open a new shell in dev container"
        echo "  format      - Format code with clang-format"
        echo "  lint        - Run static analysis with clang-tidy"
        echo "  clean       - Clean build artifacts"
        echo "  logs        - Show container logs"
        echo "  help        - Show this help message"
        ;;
esac
