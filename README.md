# Cuphead Data Generator

Cuphead Data Generator is a tool that captures real-time Cuphead gameplay, detects key game elements and outputs structured data that can later drive AI agents, automation scripts, or reinforcement learning experiments.

## Data Generator

Data Generator provides real-time feature extraction from the Cuphead window using computer vision and ONNX models.

Main features:
- Real-time capture (GDI-based).
- Entity detection: YOLO11n model (ONNX) for player, boss, projectiles, and parryables.
- HP classifier: Small CNN recognizing player HP digits.
- EX meter: simple HSV-based detector counting filled EX cards.
- Hit flash detection: simple temporal differencing to detect boss damage events.
- Preview window of the features.

Demo (x2.5):
![demo_gif](./demo/cuphead-data-generator-demo.gif)

## Actuator

Module for controlling the game. Not yet implemented.

## Python Notebooks

This repository also contains python notebooks with code that was used to train models. However, the data used for training the models, is not present due to its size.
