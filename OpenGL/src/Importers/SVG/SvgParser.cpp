#include "SvgParser.h"
struct PathCommand {
    char type;
    std::vector<float> values;
};

struct Path {
    std::vector<PathCommand> commands;
};

struct SVG {
    int width;
    int height;
    std::vector<Path> paths;
};

SVG parseSVG(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    SVG svg = {};

    std::regex width_regex(R"(width=\"([\d.]+)(\w+)\")");
        std::regex height_regex(R"(height=\"([\d.]+)(\w+)\")");
        std::smatch match;

    float svg_width = 0;
    float svg_height = 0;

    if (std::regex_search(content, match, width_regex)) {
        svg_width = std::stof(match[1]);
        std::string unit = match[2];
        if (unit == "mm") svg_width *= 3.7795275591f;
        else if (unit == "cm") svg_width *= 37.795275591f;
        else if (unit == "in") svg_width *= 96;
        else if (unit == "pt") svg_width *= 1.3333333333f;
        else if (unit == "pc") svg_width *= 16;
    }

    if (std::regex_search(content, match, height_regex)) {
        svg_height = std::stof(match[1]);
        std::string unit = match[2];
        if (unit == "mm") svg_height *= 3.7795275591f;
        else if (unit == "cm") svg_height *= 37.795275591f;
        else if (unit == "in") svg_height *= 96;
        else if (unit == "pt") svg_height *= 1.3333333333f;
        else if (unit == "pc") svg_height *= 16;
    }

    svg.width = (int)svg_width;
    svg.height = (int)svg_height;

    std::regex path_regex(R"(<path[^>]*d=\"([^"]+)\")");
        auto path_begin = std::sregex_iterator(content.begin(), content.end(), path_regex);
    auto path_end = std::sregex_iterator();

    for (std::sregex_iterator i = path_begin; i != path_end; ++i) {
        Path path = {};
        std::string d = (*i)[1];
        std::regex command_regex(R"(([MmLlHhVvCcSsQqTtAaZz])|(-?[\d.]+))");
        auto command_begin = std::sregex_iterator(d.begin(), d.end(), command_regex);
        auto command_end = std::sregex_iterator();
        PathCommand command = {};
        for (std::sregex_iterator j = command_begin; j != command_end; ++j) {
            std::string token = (*j)[0];
            if (std::isalpha(token[0])) {
                if (!command.values.empty()) {
                    path.commands.push_back(command);
                    command.values.clear();
                }
                command.type = token[0];
            }
            else {
                command.values.push_back(std::stof(token));
            }
        }
        if (!command.values.empty()) {
            path.commands.push_back(command);
            command.values.clear();
        }
        svg.paths.push_back(path);
    }

    return svg;
}
float svgAngle(float ux, float uy, float vx, float vy) {
    float dot = ux * vx + uy * vy;
    float len = std::sqrt(ux * ux + uy * uy) * std::sqrt(vx * vx + vy * vy);
    float angle = std::acos(std::max(-1.0f, std::min(1.0f, dot / len)));
    if ((ux * vy - uy * vx) < 0) angle = -angle;
    return angle;
}

void drawLine(unsigned char* data, int width, int height, int x0, int y0, int x1, int y1, int desiredChannels) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            int index = (y0 * width + x0) * desiredChannels;
            data[index] = 255;
            data[index + 1] = 255;
            data[index + 2] = 255;
            if (desiredChannels == 4) data[index + 3] = 255;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawCubicBezier(unsigned char* data, int width, int height, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int desiredChannels) {
    const int STEPS = 20;
    float prevX = x0;
    float prevY = y0;

    for (int i = 1; i <= STEPS; i++) {
        float t = (float)i / STEPS;
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        float px = uuu * x0 + 3 * uu * t * x1 + 3 * u * tt * x2 + ttt * x3;
        float py = uuu * y0 + 3 * uu * t * y1 + 3 * u * tt * y2 + ttt * y3;

        drawLine(data, width, height, (int)prevX, (int)prevY, (int)px, (int)py, desiredChannels);

        prevX = px;
        prevY = py;
    }
}
void getArcCenter(float x1, float y1, float rx, float ry, float angle, float largeArcFlag, float sweepFlag, float x2, float y2, float* cx, float* cy) {
    // Calculate the middle point between the current and the final points
    float dx2 = (x1 - x2) / 2.0;
    float dy2 = (y1 - y2) / 2.0;

    // Convert angle from degrees to radians
    angle = angle * M_PI / 180.0;

    // Calculate (x1', y1')
    float x1p = cos(angle) * dx2 + sin(angle) * dy2;
    float y1p = -sin(angle) * dx2 + cos(angle) * dy2;

    // Ensure radii are large enough
    rx = fabs(rx);
    ry = fabs(ry);
    float Prx = rx * rx;
    float Pry = ry * ry;
    float Px1 = x1p * x1p;
    float Py1 = y1p * y1p;
    double d = Px1 / Prx + Py1 / Pry;
    if (d > 1) {
        rx *= sqrt(d);
        ry *= sqrt(d);
        Prx = rx * rx;
        Pry = ry * ry;
    }

    // Calculate (cx', cy')
    double sign = (largeArcFlag == sweepFlag) ? -1 : 1;
    double coef = sign * sqrt((Prx * Pry - Prx * Py1 - Pry * Px1) / (Prx * Py1 + Pry * Px1));
    float cxp = coef * ((rx * y1p) / ry);
    float cyp = coef * -((ry * x1p) / rx);

    // Calculate (cx, cy) from (cx', cy')
    *cx = cos(angle) * cxp - sin(angle) * cyp + (x1 + x2) / 2.0;
    *cy = sin(angle) * cxp + cos(angle) * cyp + (y1 + y2) / 2.0;
}

void drawPixel(unsigned char* data, int width, int height, int x, int y) {
    // Check if the point is inside the image
    if (x >= 0 && x < width && y >= 0 && y < height) {
        // Calculate the index of the pixel
        int index = (y * width + x) * 4;

        // Set the color of the pixel
        data[index] = 255;
        data[index + 1] = 255;
        data[index + 2] = 255;
        data[index + 3] = 255;
    }
}
void drawArc(unsigned char* data, int width, int height, float cx, float cy, float rx, float ry, float startAngle, float endAngle, bool isCounterClockwise) {
    // Calculate the total angle
    float totalAngle;
    if (!isCounterClockwise && endAngle <= startAngle) {
        totalAngle = endAngle + 2 * M_PI - startAngle;
    }
    else if (isCounterClockwise && startAngle <= endAngle) {
        totalAngle = startAngle + 2 * M_PI - endAngle;
    }
    else {
        totalAngle = fabs(endAngle - startAngle);
    }

    // Calculate the number of steps
    int steps = ceil(totalAngle / (M_PI / 180.0));

    // Draw the arc
    for (int i = 0; i <= steps; i++) {
        // Calculate the current angle
        float t = i / (float)steps;
        if (!isCounterClockwise) t = 1 - t;
        float angle = startAngle + t * totalAngle;

        // Calculate the current point
        int x = cx + rx * cos(angle);
        int y = cy + ry * sin(angle);

        // Draw the point
        drawPixel(data, width, height, x, y);
    }
}
void rasterizeSVG(const SVG& svg, unsigned char* data, int desiredChannels) {
    // Set all pixels to transparent black
    for (int i = 0; i < svg.width * svg.height * desiredChannels; i += desiredChannels) {
        data[i] = 0;
        data[i + 1] = 0;
        data[i + 2] = 0;
        if (desiredChannels == 4) data[i + 3] = 0;
    }

    // Rasterize each path
    for (const Path& path : svg.paths) {
        float x = 0;
        float y = 0;
        float startX = 0;
        float startY = 0;

        PathCommand prevCommand{};
        float prevX{};
        float prevY{};
        float prevX1{};
        float prevY1{};
        float prevX2{};
        float prevY2{};
        for (const PathCommand& command : path.commands) {
            switch (command.type) {
            case 'A': {
                // Get the values from the command
                float rx = command.values[0];
                float ry = command.values[1];
                float xAxisRotation = command.values[2];
                float largeArcFlag = command.values[3];
                float sweepFlag = command.values[4];
                float x2 = command.values[5];
                float y2 = command.values[6];

                // Calculate the center of the ellipse
                float cx, cy;
                getArcCenter(x, y, rx, ry, xAxisRotation, largeArcFlag, sweepFlag, x2, y2, &cx, &cy);

                // Calculate the start and end angles
                float startAngle = atan2(y - cy, x - cx);
                float endAngle = atan2(y2 - cy, x2 - cx);

                // Determine the direction of the arc
                bool isCounterClockwise = (sweepFlag == 0);

                // Draw the arc
                drawArc(data, svg.width, svg.height, cx, cy, rx, ry, startAngle, endAngle, isCounterClockwise);

                // Update the current position
                x = x2;
                y = y2;
                break;
            }
            case 'C': {
                float x1 = command.values[0];
                float y1 = command.values[1];
                float x2 = command.values[2];
                float y2 = command.values[3];
                float x3 = command.values[4];
                float y3 = command.values[5];

                const int STEPS = 20;
                for (int i = 0; i <= STEPS; i++) {
                    float t = (float)i / STEPS;
                    float u = 1 - t;
                    float tt = t * t;
                    float uu = u * u;
                    float uuu = uu * u;
                    float ttt = tt * t;

                    float px = uuu * x + 3 * uu * t * x1 + 3 * u * tt * x2 + ttt * x3;
                    float py = uuu * y + 3 * uu * t * y1 + 3 * u * tt * y2 + ttt * y3;

                    if (i > 0) {
                        drawLine(data, svg.width, svg.height, (int)prevX, (int)prevY, (int)px, (int)py, desiredChannels);
                    }

                    prevX = px;
                    prevY = py;
                }

                x = x3;
                y = y3;
                break;
            }
            case 'M':
                x = command.values[0];
                y = command.values[1];
                startX = x;
                startY = y;
                break;
            case 'L':
                drawLine(data, svg.width, svg.height, (int)x, (int)y, (int)command.values[0], (int)command.values[1], desiredChannels);
                x = command.values[0];
                y = command.values[1];
                break;
            case 'H':
                drawLine(data, svg.width, svg.height, (int)x, (int)y, (int)command.values[0], (int)y, desiredChannels);
                x = command.values[0];
                break;
            case 'Q': {
                float x1 = command.values[0];
                float y1 = command.values[1];
                float x2 = command.values[2];
                float y2 = command.values[3];

                const int STEPS = 20;
                for (int i = 0; i <= STEPS; i++) {
                    float t = (float)i / STEPS;
                    float u = 1 - t;

                    float px = u * u * x + 2 * u * t * x1 + t * t * x2;
                    float py = u * u * y + 2 * u * t * y1 + t * t * y2;

                    if (i > 0) {
                        drawLine(data, svg.width, svg.height, (int)prevX, (int)prevY, (int)px, (int)py, desiredChannels);
                    }

                    prevX = px;
                    prevY = py;
                }

                prevX1 = x1;
                prevY1 = y1;

                x = x2;
                y = y2;
                break;
            }
            case 'S': {
                float x2 = command.values[0];
                float y2 = command.values[1];
                float x3 = command.values[2];
                float y3 = command.values[3];

                float x1 = x;
                float y1 = y;
                if (prevCommand.type == 'C' || prevCommand.type == 'S') {
                    x1 = x + (x - prevX2);
                    y1 = y + (y - prevY2);
                }

                const int STEPS = 20;
                for (int i = 0; i <= STEPS; i++) {
                    float t = (float)i / STEPS;
                    float u = 1 - t;
                    float tt = t * t;
                    float uu = u * u;
                    float uuu = uu * u;
                    float ttt = tt * t;

                    float px = uuu * x + 3 * uu * t * x1 + 3 * u * tt * x2 + ttt * x3;
                    float py = uuu * y + 3 * uu * t * y1 + 3 * u * tt * y2 + ttt * y3;

                    if (i > 0) {
                        drawLine(data, svg.width, svg.height, (int)prevX, (int)prevY, (int)px, (int)py, desiredChannels);
                    }

                    prevX = px;
                    prevY = py;
                }

                prevX2 = x2;
                prevY2 = y2;

                x = x3;
                y = y3;
                break;
            }
            case 'T': {
                float x2 = command.values[0];
                float y2 = command.values[1];

                float x1 = x;
                float y1 = y;
                if (prevCommand.type == 'Q' || prevCommand.type == 'T') {
                    x1 = x + (x - prevX1);
                    y1 = y + (y - prevY1);
                }

                const int STEPS = 20;
                for (int i = 0; i <= STEPS; i++) {
                    float t = (float)i / STEPS;
                    float u = 1 - t;

                    float px = u * u * x + 2 * u * t * x1 + t * t * x2;
                    float py = u * u * y + 2 * u * t * y1 + t * t * y2;

                    if (i > 0) {
                        drawLine(data, svg.width, svg.height, (int)prevX, (int)prevY, (int)px, (int)py, desiredChannels);
                    }

                    prevX = px;
                    prevY = py;
                }

                prevX1 = x1;
                prevY1 = y1;

                x = x2;
                y = y2;
                break;
            }
            case 'V':
                drawLine(data, svg.width, svg.height, (int)x, (int)y, (int)x, (int)command.values[0], desiredChannels);
                y = command.values[0];
                break;
            case 'Z':
                drawLine(data, svg.width, svg.height, (int)x, (int)y, (int)startX, (int)startY, desiredChannels);
                x = startX;
                y = startY;
                break;
            }
        }
    }
}
unsigned char* LoadSVG(const char* filename, int* width, int* height, int* bpp, int desiredChannels) {
    SVG svg = parseSVG(filename);
    if (svg.width == 0 || svg.height == 0) {
        std::cerr << "Error: Could not parse SVG file " << filename << std::endl;
        return nullptr;
    }

    *width = svg.width;
    *height = svg.height;
    *bpp = 8;

    unsigned char* image_data = new unsigned char[(*width) * (*height) * desiredChannels];
    if (!image_data) {
        std::cerr << "Error: Could not allocate memory for image data" << std::endl;
        return nullptr;
    }

    rasterizeSVG(svg, image_data, desiredChannels);

    return image_data;
}