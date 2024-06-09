
# Desktop Capture

## Introduction

Desktop capture software using Windows Graphics API.

## Demo



## Features

    - Select a capture monitor
    - Select capture area
    - 4K upscaling function using NIS

## Usage

    1. Select a capture monitor.  
    2. Select capture area, if you need.  
    3. Select the capture mode, Default or 4K upscaling.  
    4. Start capture.  

    Please close window when you want to finish.  

## Build

``` bash
cmake -S . -B build  
cmake --build build  
```

## Dependency

    - Windows10/11  
    - DirectX 11  
    - Windows Graphics API  
    - NIS(NVIDIA Image Scaling)  

## Reference

The capture area selection function is based on wcap.  
[https://github.com/mmozeiko/wcap](https://github.com/mmozeiko/wcap)


