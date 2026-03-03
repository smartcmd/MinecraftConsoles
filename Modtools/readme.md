# PCK Decompiler

Technical Documentation and Project Specifications

---

## Overview

**PCK Decompiler** is an open-source Python tool designed to extract and rebuild `.pck` files containing image assets (PNG) used in game UI systems.

---

## Architecture

The tool is composed of:

- `pck_tool.py` → Main application
- Graphical Interface built with `CustomTkinter`
- Drag & Drop support via `tkinterdnd2`
- Image handling via `Pillow`

It supports:

- Drag & Drop `.pck` files
- Folder-wide decompilation
- PNG injection
- Language switching (UI localization)

---

## Installation

### 1️⃣ Requirements

- Python 3.9+
- Visual Studio Code

---

### 2️⃣ Install Dependencies (Recommended via VS Code Terminal)

Open the project folder in **Visual Studio Code**.

Then open the integrated terminal:

Terminal → New Terminal

Run the following commands:

python -m venv venv

Activate the virtual environment:

Windows:

venv\\Scripts\\activate

Install required packages:

pip install --upgrade pip
pip install customtkinter pillow tkinterdnd2

---

## Running the Tool (via VS Code Terminal)

With the virtual environment activated inside VS Code terminal:

python pck_tool.py

The GUI will launch automatically.

---

## Features

### Drag & Drop

Drop `.pck` files directly into the application window to extract contents.

### Folder Decompilation

Select a folder and the tool will process all `.pck` files inside it.

### PNG Injection

Import custom `.png` files and rebuild the `.pck` structure.

### Language Selection

Switch UI language dynamically from the interface.

---

## Repository Integration

This tool is designed to:

- Live inside another repository
- Be submitted as a Pull Request
- Not require independent directory generation
- Not auto-create standalone project folders

It behaves as an internal utility module.

---

## Known Limitations

- Supports PNG-based PCK structures
- Does not reverse proprietary compression (if present)
- Assumes consistent internal file structure
