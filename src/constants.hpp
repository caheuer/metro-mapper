#pragma once

// Constants defined here are used in multiple files

const int WINDOW_WIDTH = 800; // Width of the window in pixels
const int WINDOW_HEIGHT = 600; // Height of the window in pixels

//Having different width and height for the cells actually breaks some of the rendering right now
const int CELL_WIDTH = 60; // Width of each cell in the grid in pixels
const int CELL_HEIGHT = 60; // Height of each cell in the grid in pixels

const int EXPORT_PADDING = 100; // Padding for exporting the map in pixels

const float MIN_ZOOM = 0.5f; // Minimum zoom level
const float MAX_ZOOM = 10.0f; // Maximum zoom level
const float ZOOM_STEP = 1.1f; // Zoom step factor

const int LINE_THICKNESS = 10; // Thickness of the lines in pixels

const int FONT_SIZE = 64; // Font size for text rendering

const float EPSILON = 0.0001f; // Small value for floating point comparison
const int MAX_ITERATIONS = 1000; // Maximum iterations for positioning algorithm
const int MIN_DIST = 1; // Minimum distance between stations (in pixels)