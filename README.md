<p align="center">
  <h3 align="center">Cowboy Engine</h3>
  <p align="center">
    OpenGL 4.6 Rendering Engine - Computer Graphics student project @ Paul Sabatier University - :fr:
  </p>
  <div align="center">
    <img src="https://img.shields.io/badge/stability-experimental-orange.svg"/>
    <img src="https://img.shields.io/github/license/Naereen/StrapDown.js.svg"/>
  </div>
</p>


<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#installation">Installation</a></li>
	<li><a href="#usage">Usage</a></li>
      </ul>
    </li>
    <li><a href="#results">Results</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#references">References</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

## About The Project
To assist me with my Master's degree in Computer Graphics, I implemented an OpenGL 4.6 renderer in C++ from scratch. In this post, I will present a summary of what I implemented :

- **Blinn-Phong Reflections**
- **Shadow Mapping**
- **Tensor Product B-Splines Surfaces**
- **Meshes Subdivision**
- **Implicit surfaces**
- **Skeletal Animation**
- **.glTF Loader 🚧**

#### Built With
* [OpenMesh](https://www.graphics.rwth-aachen.de/software/openmesh) Generic and efficient data structure for representing and manipulating polygonal meshes.
* [GLM](https://github.com/g-truc/glm) Header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
* [tinygltf](https://github.com/syoyo/tinygltf) Header only C++11 glTF 2.0 library.

## Getting Started
### Installation
1. Clone the repo with the submodules
   ```sh
   git clone --recurse-submodules https://github.com/Trietch/cowboy-engine.git
   ```
2. Compile
   ```sh
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j
   ```

### Usage
1. Execute the `cowboy-engine` binary file you just compiled
   ```sh
   ./cowboy-engine
   ```

## Screenshots
<div align="center">


</div>

### Blinn-Phong Reflections

![blinnphong](https://user-images.githubusercontent.com/1809578/135022371-b950902d-e367-4f69-99aa-43bec3f6aba6.png)

### Shadow Mapping

![shadowmap](https://user-images.githubusercontent.com/1809578/135022387-581cc4aa-653f-4ff0-ac00-98d43a369b2e.png)

### Tensor Product B-Splines Surfaces

https://user-images.githubusercontent.com/1809578/135022406-267ad51f-3de3-42d2-a7df-45b3fa884504.mp4

![bsplines](https://user-images.githubusercontent.com/1809578/135022397-c54d7e4b-37ec-4d8a-a4e4-c1177835bc5a.png)

![tensor](https://user-images.githubusercontent.com/1809578/135022414-cf380871-9686-400b-86cc-6a9e0ce81851.png)

### Meshes Subdivision

![subdivision-normals](https://user-images.githubusercontent.com/1809578/135022420-8783cb61-97b5-40cd-a3b5-efa7afab97f1.png)

### Implicit surfaces

https://user-images.githubusercontent.com/1809578/135022431-1ad40386-4f7c-4424-bfd2-040ca6f33dc0.mp4

### Skeletal Animation

![bones](https://user-images.githubusercontent.com/1809578/135022440-015a25e2-4df6-416c-b332-0abddeeba7d6.png)


## Contact
 » [contact@tmarrec.dev](mailto:contact@tmarrec.dev)

## License
Distributed under the MIT License. See `LICENSE` for more information.
