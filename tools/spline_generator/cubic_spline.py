import pygame
import sys
import numpy as np
from scipy.interpolate import (
    CubicSpline,
    interp1d,
    BarycentricInterpolator,
    KroghInterpolator,
)


def cubic_spline_interpolation(points, num_samples=200):
    """
    Create a cubic spline interpolation through the given points.
    Returns x and y arrays of the interpolated curve.
    """
    if len(points) < 2:
        return np.array([]), np.array([])

    # Extract x and y coordinates
    x_points = np.array([p[0] for p in points])
    y_points = np.array([p[1] for p in points])

    if len(points) == 2:
        # For two points, just return a line
        x_interp = np.linspace(x_points[0], x_points[1], num_samples)
        y_interp = np.linspace(y_points[0], y_points[1], num_samples)
        return x_interp, y_interp

    # For 3 or more points, use different interpolation methods
    try:
        # Generate interpolated points
        x_interp = np.linspace(x_points[0], x_points[-1], num_samples)

        cs = CubicSpline(x_points, y_points)
        y_interp = cs(x_interp)

        return x_interp, y_interp
    except:
        # Fallback to linear interpolation if interpolation fails
        x_interp = np.linspace(x_points[0], x_points[-1], num_samples)
        y_interp = np.interp(x_interp, x_points, y_points)
        return x_interp, y_interp


class CubicSplinePlayground:
    def __init__(self, width=800, height=600):
        pygame.init()
        self.width = width
        self.height = height
        self.screen = pygame.display.set_mode((width, height), pygame.RESIZABLE)
        pygame.display.set_caption("Cubic Spline Playground")

        self.control_points = [
            # initial points in bottom-left and top-right corners
            (0.0, 0.0),  # Bottom-left corner
            (1.0, 1.0),  # Top-right corner
        ]

        # Check for command line argument to load a file
        if len(sys.argv) > 1:
            self.load_curve_from_file(sys.argv[1])

        self.dragging_point = None
        self.dragging_offset = (0, 0)

        # Colors
        self.bg_color = (40, 40, 40)
        self.grid_color = (80, 80, 80)
        self.diagonal_color = (120, 120, 120)
        self.spline_color = (100, 200, 255)
        self.point_color = (255, 100, 100)
        self.point_hover_color = (255, 150, 150)

        self.point_radius = 8
        self.clock = pygame.time.Clock()

    def screen_to_curve_coords(self, pos):
        """Convert screen coordinates to curve coordinates (0-1 range)"""
        x, y = pos
        return (x / self.width, 1.0 - y / self.height)

    def curve_to_screen_coords(self, pos):
        """Convert curve coordinates to screen coordinates"""
        x, y = pos
        return (int(x * self.width), int((1.0 - y) * self.height))

    def draw_grid(self):
        """Draw grid lines"""
        # Vertical lines
        for i in range(1, 10):
            x = i * self.width // 10
            pygame.draw.line(self.screen, self.grid_color, (x, 0), (x, self.height), 1)

        # Horizontal lines
        for i in range(1, 10):
            y = i * self.height // 10
            pygame.draw.line(self.screen, self.grid_color, (0, y), (self.width, y), 1)

    def get_point_at_position(self, pos):
        """Check if there's a control point at the given position"""
        for i, point in enumerate(self.control_points):
            screen_pos = self.curve_to_screen_coords(point)
            distance = (
                (pos[0] - screen_pos[0]) ** 2 + (pos[1] - screen_pos[1]) ** 2
            ) ** 0.5
            if distance <= self.point_radius:
                return i
        return None

    def add_control_point(self, pos):
        """Add a control point at the given position"""
        curve_pos = self.screen_to_curve_coords(pos)
        # Clamp to valid range
        curve_pos = (max(0, min(1, curve_pos[0])), max(0, min(1, curve_pos[1])))
        self.control_points.append(curve_pos)
        # Sort points by x coordinate
        self.control_points.sort(key=lambda p: p[0])

    def remove_control_point(self, index):
        """Remove a control point at the given index"""
        if 0 <= index < len(self.control_points):
            del self.control_points[index]

    def draw_control_points(self):
        """Draw all control points"""
        mouse_pos = pygame.mouse.get_pos()

        for point in self.control_points:
            screen_pos = self.curve_to_screen_coords(point)
            distance = (
                (mouse_pos[0] - screen_pos[0]) ** 2
                + (mouse_pos[1] - screen_pos[1]) ** 2
            ) ** 0.5

            # Use hover color if mouse is near
            color = (
                self.point_hover_color
                if distance <= self.point_radius
                else self.point_color
            )

            pygame.draw.circle(self.screen, color, screen_pos, self.point_radius)
            pygame.draw.circle(
                self.screen, (255, 255, 255), screen_pos, self.point_radius, 2
            )

    def load_curve_from_file(self, filename):
        """Load control points from a file"""
        try:
            with open(filename, "r") as f:
                lines = f.readlines()

            if not lines:
                print(f"Error: File {filename} is empty")
                return

            # Parse header: <number of control points> <number of samples>
            header = lines[0].strip().split()
            if len(header) != 2:
                print(f"Error: Invalid file format in {filename}")
                return

            num_control_points = int(header[0])
            num_samples = int(header[1])

            if len(lines) < 1 + num_control_points:
                print(f"Error: Not enough control point data in {filename}")
                return

            # Load control points
            control_points = []
            for i in range(1, 1 + num_control_points):
                point_data = lines[i].strip().split()
                if len(point_data) != 2:
                    print(
                        f"Error: Invalid control point format at line {i+1} in {filename}"
                    )
                    return
                x, y = float(point_data[0]), float(point_data[1])
                control_points.append((x, y))

            self.control_points = control_points
            print(f"Loaded {len(control_points)} control points from {filename}")

        except FileNotFoundError:
            print(f"Error: File {filename} not found")
        except ValueError as e:
            print(f"Error parsing file {filename}: {e}")
        except Exception as e:
            print(f"Error loading file {filename}: {e}")

    def handle_events(self):
        """Handle pygame events"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    self.save_curve_to_file()

            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:  # Left mouse button
                    point_index = self.get_point_at_position(event.pos)
                    if point_index is not None:
                        # Start dragging existing point
                        self.dragging_point = point_index
                        screen_pos = self.curve_to_screen_coords(
                            self.control_points[point_index]
                        )
                        self.dragging_offset = (
                            event.pos[0] - screen_pos[0],
                            event.pos[1] - screen_pos[1],
                        )
                    else:
                        # Add new point and start dragging it immediately
                        curve_pos = self.screen_to_curve_coords(event.pos)
                        curve_pos = (
                            max(0, min(1, curve_pos[0])),
                            max(0, min(1, curve_pos[1])),
                        )
                        self.control_points.append(curve_pos)
                        self.control_points.sort(key=lambda p: p[0])

                        # Find the index of the newly added point and start dragging
                        for i, point in enumerate(self.control_points):
                            if point == curve_pos:
                                self.dragging_point = i
                                break

                        # Set dragging offset to zero since we're starting from the exact click position
                        self.dragging_offset = (0, 0)

                elif event.button == 3:  # Right mouse button
                    point_index = self.get_point_at_position(event.pos)
                    if point_index is not None:
                        self.remove_control_point(point_index)

            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:  # Left mouse button
                    self.dragging_point = None

            elif event.type == pygame.MOUSEMOTION:
                if self.dragging_point is not None:
                    # Update dragged point position
                    new_pos = (
                        event.pos[0] - self.dragging_offset[0],
                        event.pos[1] - self.dragging_offset[1],
                    )
                    curve_pos = self.screen_to_curve_coords(new_pos)
                    # Clamp to valid range
                    curve_pos = (
                        max(0, min(1, curve_pos[0])),
                        max(0, min(1, curve_pos[1])),
                    )
                    self.control_points[self.dragging_point] = curve_pos
                    # Re-sort points by x coordinate
                    self.control_points.sort(key=lambda p: p[0])
                    # Update dragging index after sorting
                    for i, point in enumerate(self.control_points):
                        if point == curve_pos:
                            self.dragging_point = i
                            break

        return True

    def draw_spline(self):
        """Draw the cubic spline curve through all control points"""
        if len(self.control_points) < 2:
            return

        # Get interpolated curve points
        x_curve, y_curve = cubic_spline_interpolation(self.control_points)

        if len(x_curve) == 0:
            return

        # Convert curve coordinates to screen coordinates and draw
        screen_points = []
        for i in range(len(x_curve)):
            screen_pos = self.curve_to_screen_coords((x_curve[i], y_curve[i]))
            screen_points.append(screen_pos)

        # Draw the curve as connected line segments
        if len(screen_points) > 1:
            pygame.draw.lines(self.screen, self.spline_color, False, screen_points, 3)

    def draw(self):
        """Draw everything"""
        self.screen.fill(self.bg_color)
        self.draw_grid()
        self.draw_spline()
        self.draw_control_points()
        pygame.display.flip()

    def save_curve_to_file(self):
        """Save the current curve to a .txt file with control points and 1000 samples"""
        if len(self.control_points) < 2:
            print("Not enough control points to save curve (need at least 2)")
            return

        # Get curve data with 1000 samples
        x_curve, y_curve = cubic_spline_interpolation(
            self.control_points, num_samples=1000
        )

        if len(x_curve) == 0:
            print("Failed to generate curve data")
            return

        # Generate filename with timestamp
        import datetime

        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"curve_{timestamp}.txt"

        try:
            with open(filename, "w") as f:
                # Write header: <number of control points> <number of samples>
                f.write(f"{len(self.control_points)} {len(y_curve)}\n")

                # Write control points (x y pairs)
                for x, y in self.control_points:
                    f.write(f"{x:.6f} {y:.6f}\n")

                # Write sample values (y values only)
                for y_val in y_curve:
                    f.write(f"{y_val:.6f}\n")

            print(f"Curve saved successfully!")
            print(f"  File: {filename}")
            print(f"  Control points: {len(self.control_points)}")
            print(f"  Samples: {len(y_curve)}")
            print(f"  Y range: [{y_curve.min():.6f}, {y_curve.max():.6f}]")

        except Exception as e:
            print(f"Error saving curve: {e}")

    def run(self):
        """Main game loop"""
        running = True
        while running:
            running = self.handle_events()
            self.draw()
            self.clock.tick(60)

            # get latest window size
            self.width, self.height = pygame.display.get_surface().get_size()

        pygame.quit()
        sys.exit()


if __name__ == "__main__":
    playground = CubicSplinePlayground()
    playground.run()
