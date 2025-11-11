# Cuphead Data Generator

Cuphead Data Generator is a tool that captures real-time Cuphead gameplay, detects key game elements and outputs structured data that can later drive AI agents, automation scripts, or reinforcement learning experiments.

## Data Generator

### Overview

Data Generator provides real-time feature extraction from the Cuphead window using computer vision and ONNX models.

Main features:
- Real-time capture (GDI-based).
- Entity detection: YOLO11n model (ONNX) for player, boss, projectiles, and parryables.
- HP classifier: small CNN recognizing player HP digits.
- EX meter: simple HSV-based detector counting filled EX cards.
- Hit flash detection: simple temporal differencing to detect boss damage events.
- Preview window of the features.

### Demo 

Below you can see demo of the data generator (x2.5).

In case the GIF is slow, you can view the demo [at this link](https://drive.google.com/file/d/1-SeAOhm8YXm10nTiWxYYODNv5oR4WmFC/view?usp=sharing). 

![demo_gif](./demo/cuphead-data-generator-demo.gif)

### Limitations

The Data Generator was developed and tested only on the first level of Cuphead.
The model's generalization to later levels has not been verified and will likely require additional training of the entity detection network.

## Actuator

Module for controlling the game. Not yet implemented.

## Python Notebooks

This repository also contains python notebooks with code that was used to train models. However, the data used for training the models, is not present due to its size.
