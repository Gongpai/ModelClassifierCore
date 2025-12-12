# **Model Classifier Core — Unreal Engine Plugin (Alpha)**

**Model Classifier Core** is an experimental **Unreal Engine plugin** designed to automatically identify what a **3D model asset** represents using an AI-based classification pipeline.
The goal is to eliminate repetitive manual work when matching 3D item assets with item “state data”—especially in large projects with hundreds or thousands of assets.

This plugin is currently in **Alpha**.
Many features are incomplete, experimental, or subject to change.
Documentation, UI tools, and testing utilities will be added in future updates.

---

## 🚧 **Alpha Notice**

This repository contains an **active, in-progress** version of the plugin.

* No user interface is available yet.
* No public testing tools (e.g., drag-and-drop image preview) are implemented.
* No user documentation or full setup guide is available.
* The pipeline is functional, but internal behavior may change.

If you are reviewing or experimenting with the project, please note that the codebase is still evolving rapidly.

---

# ⭐ **What the Plugin Does**

The plugin analyzes a **3D model** (either from a `.fbx/.obj` file or an Unreal static mesh asset) and predicts what object it represents by:

1. **Rendering the model** into an image.
2. Feeding that image into a **CLIP model** running through **Unreal Engine’s NNE (Neural Network Engine)**.
3. Comparing the image features against a database of **text features** generated from ImageNet classes (or custom prompts).
4. Producing a label that represents what the model “looks like.”

This enables automated workflows such as:

* Auto-assigning metadata or item states to assets
* Quickly scanning and organizing large 3D asset libraries
* Automatically tagging imported assets
* Assisting pipelines for games, virtual production, or digital asset management

---

# 🧠 **Behind the Scenes — Core Technologies**

The plugin combines several technologies to build a complete model → render → classify pipeline:

---

## **3D Import & Mesh Processing**

* **FBX SDK**
  Used to import raw `.fbx` files when available.

* **Assimp**
  Used for parsing `.obj`/`.fbx` geometry, materials, and mesh structure when working with raw model files.
  If the Unreal asset still has its source file on disk, Assimp is used to extract the highest-fidelity data possible.

* **Unreal Asset Mesh Extraction**
  If no source file exists, the plugin extracts mesh data directly from the Unreal asset.
  *(Limitations: textures, materials, and some metadata cannot be recovered due to Unreal’s API restrictions.)*

---

## **Rendering Pipeline**

* **OpenGL Off-screen Rendering**
  Used to render the mesh into an image buffer without requiring Unreal’s renderer.
  This allows consistent results across platforms and independent of viewport states.

* **stb_image_write (for debugging)**
  Used to save `.png` files to verify rendering output during development.
  In the production pipeline, images remain only in memory.

The actual pipeline keeps rendered images entirely in memory using:
`std::vector<unsigned char> (RGBA8)`

---

## **AI Classification Pipeline**

* Images in memory are passed directly into Unreal’s **NNE (Neural Network Engine)**.

* CLIP image encoder & text encoder generate:

  * **Image features**
  * **Text features** (generated from ImageNet’s 1,000 classes × 5 prompt variations, minus exclusions → **4,985 prompts**)

* Cosine similarity identifies the closest matching class.

Because the plugin targets **rendered images**, classifications inherently understand that the input is *not a real photograph*—the pipeline is built around synthetic images.

---

# 🔧 **Current Capabilities**

✔ Load and parse mesh data from Unreal assets or raw model files
✔ Off-screen OpenGL rendering to image memory
✔ AI inference with NNE + CLIP
✔ ImageNet-based prediction via cosine similarity
✔ Full end-to-end model → label pipeline

---

# 🚀 **Planned Features (Roadmap)**

* **User-friendly UI panel** inside Unreal Editor

  * Drop a mesh → preview render → view predicted labels
* **Batch-processing tools**

  * Automatically classify folders of assets
* **Custom prompt editor**

  * Add domain-specific vocabulary
* **Documentation & Quick Start Guide**
* **Improved material/texture rendering** (limited by UE API)
* **Better pre-alignment of meshes before rendering**

---

# 📂 **Repository Status**

This repository is intended for early testers and developers who want to understand or contribute to the pipeline.
A stable public version will be released when:

* UI tools are complete
* Documentation is available
* Test scenes and examples are included

---

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/topics/git/add_files/#add-files-to-a-git-repository) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin https://git.kohtnas.com/publicgroup/modelclassifiercore.git
git branch -M main
git push -uf origin main
```
