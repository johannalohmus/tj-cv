// Johanna Lohmus
// Period 5

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <math.h> 
#include <cstdlib>
#include <time.h>
#include <list>
#include <cstring>
#include <algorithm>
#include <vector>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <map>

using namespace std;

// class for pixels
class Pixel
{
    public:
    int get_red()
    {
        return red;
    }
    int get_green()
    {
        return green;
    }
    int get_blue()
    {
        return blue;
    }
    void set_red(int new_red)
    {
        red = new_red;
    }
    void set_green(int new_green)
    {
        green = new_green;
    }
    void set_blue(int new_blue)
    {
        blue = new_blue;
    }

    private:
    int red;
    int green;
    int blue;
};

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

// sets all RGB values in a pixel to given number
Pixel set_all(int num)
{
    Pixel pxl;
    pxl.set_red(num);
    pxl.set_green(num);
    pxl.set_blue(num);
    return pxl;
}

vector<vector<Pixel>> input;
vector<vector<Pixel>> grayscale, gx, gy, magnitude, edge, h1, h2, angles, suppression, combined, votes, unrounded_angles, centers, identified_circles;
int height, width;

// gets grayscale image, returns new vector of vectors
vector<vector<Pixel>> getGrayscale(vector<vector<Pixel>> coloredscale, int height, int width)
{ 
    vector<vector<Pixel>> grayscale;

    // goes through each pixel in vector of vectors and gets average of RGB values
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel colored_pixel, gray_pixel;
            colored_pixel = coloredscale[i][j];

            int avg = (colored_pixel.get_red() + colored_pixel.get_green() + colored_pixel.get_blue())/3;
            gray_pixel = set_all(avg);

            row.push_back(gray_pixel);
        }
        grayscale.push_back(row);
    }
    return grayscale;
}

// returns vector of vectors of GX
vector<vector<Pixel>> getGx(vector<vector<Pixel>> &pixels, int height, int width)
{
    vector<vector<Pixel>> gx;
    // go through each pixel...
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            // if pixel is on the edge, put 0
            if (i == 0 || i == height-1 || j == 0 || j == height-1)
            {
                pixel = set_all(0);
                row.push_back(pixel);
            }

            // applies gx mask to surrounding pixels and puts the result into a new vector of vectors in the middle pixel's location
            else
            {
                int mask = pixels[i-1][j-1].get_red() * -1 + pixels[i-1][j].get_red() * 0 + pixels[i-1][j+1].get_red() * 1 + pixels[i][j-1].get_red() * -2 + pixels[i][j].get_red() * 0 + pixels[i][j+1].get_red() * 2 + pixels[i+1][j-1].get_red() * -1 + pixels[i+1][j].get_red() * 0 + pixels[i+1][j+1].get_red() * 1;
                pixel = set_all(mask);
                row.push_back(pixel);
            }
        }
        gx.push_back(row);
    }
    return gx;
}

// return vector of vectors of GY
vector<vector<Pixel>> getGy(vector<vector<Pixel>> &pixels, int height, int width)
{ 
    vector<vector<Pixel>> gy;
    // go through each pixel
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            // if pixel is on edge, put 0
            if (i == 0 || i == height-1 || j == 0 || j == height-1)
            {
                pixel = set_all(0);
                row.push_back(pixel);
            }

            // applies gy mask to surrounding pixels and puts the result to a new vector of vectors in the middle pixel's location
            else
            {
                int mask = pixels[i-1][j-1].get_red() * -1 + pixels[i-1][j].get_red() * -2 + pixels[i-1][j+1].get_red() * -1 + pixels[i][j-1].get_red() * 0 + pixels[i][j].get_red() * 0 + pixels[i][j+1].get_red() * 0 + pixels[i+1][j-1].get_red() * 1 + pixels[i+1][j].get_red() * 2 + pixels[i+1][j+1].get_red() * 1;
                pixel = set_all(mask);
                row.push_back(pixel);
            }
        }
        gy.push_back(row);
    }
    return gy;
}


// returns a new vector of vectors of pixels of the magnitude of gx and gy
vector<vector<Pixel>> getMagnitude(vector<vector<Pixel>> &gx, vector<vector<Pixel>> &gy, int height, int width)
{
    vector<vector<Pixel>> magnitude_vector;
    // go through each pixel in gx and gy
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            // get the square root of the squares of gx and gy at given position
            int m = sqrt(gx[i][j].get_red() * gx[i][j].get_red() + gy[i][j].get_red() * gy[i][j].get_red());
            pixel = set_all(m);
            row.push_back(pixel);
        }
        magnitude_vector.push_back(row);
    }
    return magnitude_vector;
}

// detects the edges given magnitude vector of vectors of pixels
vector<vector<Pixel>> edge_detection(vector<vector<Pixel>> &pixels, int height, int width, int threshold)
{
    vector<vector<Pixel>> edges;

    // go through each pixel...
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;

            // if pixel exceeds threshold, mark it as an edge with black
            if (pixels[i][j].get_red() >= threshold)
            {
                pixel = set_all(1);
                row.push_back(pixel);
            }
            // else, no edge
            else
            {
                pixel = set_all(0);
                row.push_back(pixel);
            }
        }
        edges.push_back(row);
    }
    return edges;
}

// detects the edges given magnitude vector of vectors of pixels
vector<vector<Pixel>> double_threshold(vector<vector<Pixel>> &pixels, int height, int width, int lower_t, int upper_t)
{
    vector<vector<Pixel>> edges;

    // go through each pixel...
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;

            // if pixel exceeds threshold, mark it as an edge with black
            if (pixels[i][j].get_red() >= upper_t)
            {
                pixel = set_all(2);
                row.push_back(pixel);
            }
            // else, no edge
            else if (pixels[i][j].get_red() >= lower_t && pixels[i][j].get_red() < upper_t)
            {
                pixel = set_all(1);
                row.push_back(pixel);
            }
            else
            {
                pixel = set_all(0);
                row.push_back(pixel);
            }
        }
        edges.push_back(row);
    }
    return edges;
}

void recurse(int i, int j, int height, int width)
{
    // if its not a 1 just return
    if(edge[i][j].get_red() != 1)
    {
        return;
    }
    // if it is a 1, change it to a 3 to keep track of what has already been visited and then recurse on it in all directions
    else if (edge[i][j].get_red() == 1)
    {
        Pixel pixel;
        pixel = set_all(3);
        edge[i][j] = pixel;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                if (x != 0 && y != 0 && i+x >= 0 && j+y >= 0 && i+x < height && j+y < width)
                {
                    recurse(i+x, j+y, height, width);
                }
            }
        }
    }

}

vector<vector<Pixel>> hysteresis(int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // for every 2 recurse on it
            if (edge[i][j].get_red() == 2)
            {
                for (int x = -1; x < 2; x++)
                {
                    for (int y = -1; y < 2; y++)
                    {
                        // recurse in all 8 directions
                        if (x != 0 && y != 0)
                        {
                            recurse(i+x, j+y, height, width);
                        }
                    }
                }
            }
        }
    }
    return edge;

}

vector<vector<Pixel>> replace(vector<vector<Pixel>> pixels, int height, int width)
{
    Pixel a, b;
    a = set_all(0);
    b = set_all(1);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // if the pixel is a 1, it gets converted to a 0 (no edge)
            if (pixels[i][j].get_red() == 1)
            {
                pixels[i][j] = a;
            }
            // if the pixel is either 2 or 3, then there is an edge
            if (pixels[i][j].get_red() == 2 || pixels[i][j].get_red() == 3)
            {
                pixels[i][j] = b;
            }
        }
    }
    return pixels;
}

// rounds to nearest angle
int round_angle(double a)
{
    if (a <= 180 && a > 157.5)
    {
        return 180;
    }
    else if (a <= 157.5 && a > 112.5)
    {
        return 135;
    }
    else if (a <= 112.5 && a > 67.5)
    {
        return 90;
    }
    else if (a <= 67.5 && a > 22.5)
    {
        return 45;
    }
    else if (a <= 22.5 && a > -22.5)
    {
        return 0;
    }
    else if (a <= -22.5 && a > -67.5)
    {
        return -45;
    }
    else if (a <= -67.5 && a > -112.5)
    {
        return -90;
    }
    else if (a <= -112.5 && a > -157.5)
    {
        return -135;
    }
    else if (a <= -157.5 && a >= -180)
    { 
        return -180;
    }
    return 0;
}

// gets angle and rounds
vector<vector<Pixel>> angle_vector(vector<vector<Pixel>> &gx, vector<vector<Pixel>> &gy, int height, int width)
{
    vector<vector<Pixel>> angle_vector;
    // go through each pixel in gx and gy
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            // get the angle of gy and gx using arctan
            double a = atan2(gy[i][j].get_red(), gx[i][j].get_red()) * (180 / 3.14159265);
            // round to nearest angle (-180, -135, ... 135, 180)
            int rounded = round_angle(a);

            pixel = set_all(rounded);
            row.push_back(pixel);
        }
        angle_vector.push_back(row);
    }
    return angle_vector;
}

vector<vector<Pixel>> unrounded_angle_vector(vector<vector<Pixel>> &gx, vector<vector<Pixel>> &gy, int height, int width)
{
    vector<vector<Pixel>> angle_vector;
    // go through each pixel in gx and gy
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            // get the angle of gy and gx using arctan
            double a = atan2(gy[i][j].get_red(), gx[i][j].get_red()) * 180 / 3.14159265;
            int rounded = (int)round(a);

            pixel = set_all(rounded);
            row.push_back(pixel);
        }
        angle_vector.push_back(row);
    }
    return angle_vector;
}

// checks if c is bigger than a and b in magnitude vector of vectors
vector<vector<Pixel>> non_max_suppression(vector<vector<Pixel>> &angles, vector<vector<Pixel>> &magnitude, int height, int width)
{
    vector<vector<Pixel>> suppression_vector;
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            int a, b;
            int angle = angles[i][j].get_red();
            int c = magnitude[i][j].get_red();
            if (i != 0 && i != height-1 && j != 0 && j != height-1)
            {
                if (angle == 0 || angle == 180 || angle == -180)
                {
                    a = magnitude[i][j-1].get_red();
                    b = magnitude[i][j+1].get_red();
                }
                if (angle == 90 | angle == -90)
                {
                    a = magnitude[i+1][j].get_red();
                    b = magnitude[i-1][j].get_red();
                }
                if (angle == -45 | angle == 135)
                {
                    a = magnitude[i-1][j+1].get_red();
                    b = magnitude[i+1][j-1].get_red();
                }
                if (angle == -135 | angle == 45)
                {
                    a = magnitude[i-1][j-1].get_red();
                    b = magnitude[i+1][j+1].get_red();
                }

                if (c >= b && c >= a)
                {
                    pixel = set_all(1);
                }
                else
                {
                    pixel = set_all(0);
                }
            }
            else
            {
                pixel  = set_all(0);
            }
            row.push_back(pixel);
        }
        suppression_vector.push_back(row);
    }
    return suppression_vector;
}

vector<vector<Pixel>> combined_algorithm(vector<vector<Pixel>> &dt, vector<vector<Pixel>> &nms, int height, int width)
{
    vector<vector<Pixel>> c;
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            if (dt[i][j].get_red() == 1 && nms[i][j].get_red() == 1)
            {
                pixel = set_all(1);
            }
            else
            {
                pixel = set_all(0);
            }
            row.push_back(pixel);
        }
        c.push_back(row);
    }
    return c;
}

void draw_line(int x1, int y1, int x2, int y2)
{
    int delta_x = x2 - x1;
    int delta_y = y2 - y1;

    if (abs(delta_x) >= abs(delta_y))
    {
        if (delta_x < 0)
        {
            delta_x *= -1;
            int temp = x1;
            x1 = x2;
            x2 = temp;
            int temp1 = y1;
            y1 = y2;
            y2 = temp1;
        }
        if (delta_y < 0)
        {
            delta_y *= -1;
        }

        int delta_e = abs(delta_y) - abs(delta_x);
        int i = x1;
        int j = y1;
        while (i <= x2 && i < width && i >= 0 && j < height && j >= 0)
        {
            int vote = votes[j][i].get_red();
            Pixel pixel;
            pixel = set_all(vote+1);
            votes[j][i] = pixel;
            if (delta_e >= 0)
            {
                if (y2-y1<0)
                {
                    j--;
                }
                else
                {
                    j++;
                }
                delta_e -= delta_x;
            }
            i++;
            delta_e += delta_y;
        }
    }

    else
    {
        if (delta_y < 0)
        {
            delta_y *= -1;
            int temp = y1;
            y1 = y2;
            y2 = temp;
            int temp1 = x1;
            x1 = x2;
            x2 = temp1;
        }

        if (delta_x < 0)
        {
            delta_x *= -1;
        }

        int delta_e = abs(delta_x) - abs(delta_y);
        int i = y1;
        int j = x1;
        while (i <= y2 && i < height && i >= 0 && j < width && j >= 0)
        {
            int vote = votes[i][j].get_red();
            Pixel pixel;
            pixel = set_all(vote+1);
            votes[i][j] = pixel;
            if (delta_e >= 0)
            {
               if (x2-x1<0)
                {
                    j--;
                }
                else
                {
                    j++;
                }
                delta_e -= delta_y;
            }
            i++;
            delta_e += delta_x;
        }
    }
}

vector<vector<Pixel>> voting(vector<vector<Pixel>> &nms, vector<vector<Pixel>> &ur_angles, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = set_all(0);
            row.push_back(pixel);
        }
        votes.push_back(row);
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int edge;
            int angle;
            edge = nms[i][j].get_red();
            if (edge == 1)
            {
                angle = ur_angles[i][j].get_red();
                double ax = (cos(angle * (3.1415926/180)));
                double xnew = double(j) + 10 * ax;
                double ay = (sin(angle * (3.1415926/180)));
                double ynew = double(i) + 10 * ay;
                double slope;
                if (xnew - j == 0) 
                {
                    slope = 99999;
                }
                else
                {
                    slope = (ynew - i)/(xnew - j);
                }
                double b = (i - slope * j);
                int x_if_y0 = (int)round(-b/slope); // y = 0
                int x_if_yheight = (int)round((height-b)/slope); // y = height
                int y_if_xwidth = (int)round(width*slope + b); // x = width
                int y_if_x0 = (int)round(b); //x = 0

                if (x_if_y0 >= 0 && x_if_y0 <= width && x_if_yheight >= 0 && x_if_yheight <= width)
                {
                    draw_line(x_if_y0, 0, x_if_yheight, height);
                }
                if (x_if_y0 >= 0 && x_if_y0 <= width && y_if_xwidth >= 0 && y_if_xwidth <= height)
                {
                    draw_line(x_if_y0, 0, width, y_if_xwidth);
                }
                if (x_if_y0 >= 0 && x_if_y0 <= width && y_if_x0 >= 0 && y_if_x0 <= height)
                {
                    draw_line(x_if_y0, 0, 0, y_if_x0);
                }
                if (x_if_yheight >= 0 && x_if_yheight <= width && y_if_xwidth >= 0 && y_if_xwidth <= height)
                {
                    draw_line(x_if_yheight, height, width, y_if_xwidth);
                }
                if (x_if_yheight >= 0 && x_if_yheight <= width && y_if_x0 >= 0 && y_if_x0 <= height)
                {
                    draw_line(x_if_yheight, height, 0, y_if_x0);
                }
                if (y_if_xwidth >= 0 && y_if_xwidth <= height && y_if_x0 >= 0 && y_if_x0 <= height)
                {
                    draw_line(width, y_if_xwidth, 0, y_if_x0);
                }
                //if (edgepoint1 >= 0 && edgepoint1 <= width )
            }
            
           
            
        }
    }
    
    return votes;
}

int vector_max(vector<vector<Pixel>> v, int height, int width)
{
    int max_num = 0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int pixel_val;
            pixel_val = v[i][j].get_red();
            if (pixel_val > max_num)
            {
                max_num = pixel_val;
            }
        }
    }
    return max_num;
}

void draw_circle(double r, double cx, double cy, string type)
{
    int x,  y, xmax, y2, y2_new, ty;
    xmax = (int) (r * 0.70710678);
    y = r;
    y2 = y * y;
    ty = (2 * y) - 1;
    y2_new = y2;
    for (x = 0; x <= xmax + 1; x++) 
    {
        if ((y2 - y2_new) >= ty) 
        {
            y2 -= ty;
            y -= 1;
            ty -= 2;
        }
        Pixel pixel;
        pixel.set_red(255);
        pixel.set_green(0);
        pixel.set_blue(0);
        if (cx + x > 0 && cx + x < width && cy + y > 0 && cy + y < height)
        {
            if (type == "centers")
            {
                centers[cy+y][cx+x] = pixel;
            }
            else
            {
                identified_circles[cy+y][cx+x] = pixel;
            }
        }
        
        if (cx + x > 0 && cx + x < width && cy - y > 0 && cy - y < height)
        {
            if (type == "centers")
            {
                centers[cy-y][cx+x] = pixel;
            }
            else
            {
                identified_circles[cy-y][cx+x] = pixel;
            }
        }

        if (cx - x > 0 && cx - x < width && cy + y > 0 && cy + y < height)
        {
            if (type == "centers")
            {
                centers[cy+y][cx-x] = pixel;
            }
            else
            {
                identified_circles[cy+y][cx-x] = pixel;
            }
        }

        if (cx - x > 0 && cx - x < width && cy - y > 0 && cy - y < height)
        {
            if (type == "centers")
            {
                centers[cy-y][cx-x] = pixel;
            }
            else
            {
                identified_circles[cy-y][cx-x] = pixel;
            }
        }

        if (cx + y > 0 && cx + y < width && cy + x > 0 && cy + x < height)
        {
            if (type == "centers")
            {
                centers[cy+x][cx+y] = pixel;
            }
            else
            {
                identified_circles[cy+x][cx+y] = pixel;
            }
        }

        if (cx + y > 0 && cx + y < width && cy - x > 0 && cy - x < height)
        {
            if (type == "centers")
            {
                centers[cy-x][cx+y] = pixel;
            }
            else
            {
                identified_circles[cy-x][cx+y] = pixel;
            }
        }

        if (cx - y > 0 && cx - y < width && cy + x > 0 && cy + x < height)
        {
            if (type == "centers")
            {
                centers[cy+x][cx-y] = pixel;
            }
            else
            {
                identified_circles[cy+x][cx-y] = pixel;
            }
        }

        if (cx - y > 0 && cx - y < width && cy - x > 0 && cy - x < height)
        {
            if (type == "centers")
            {
                centers[cy-x][cx-y] = pixel;
            }
            else
            {
                identified_circles[cy-x][cx-y] = pixel;
            }
        }
        
        y2_new -= (2 * x) - 3;
    }
}

vector<vector<Pixel>> find_centers(int t, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            int copy_red = input[i][j].get_red();
            int copy_green = input[i][j].get_green();
            int copy_blue = input[i][j].get_blue();
            Pixel pixel;
            pixel.set_blue(copy_blue);
            pixel.set_green(copy_green);
            pixel.set_red(copy_red);
            row.push_back(pixel);
        }
        centers.push_back(row);
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (votes[i][j].get_red() >= t)
            {
                draw_circle(1, j, i, "centers");
                draw_circle(2, j, i, "centers");
                draw_circle(3, j, i, "centers");
                draw_circle(4, j, i, "centers");
            }
        }
    }
    return centers;
}

double count_circle(double r, double cx, double cy)
{
    double edge_count, all_count;
    int x,  y, xmax, y2, y2_new, ty;
    xmax = (int) (r * 0.70710678);
    y = r;
    y2 = y * y;
    ty = (2 * y) - 1;
    y2_new = y2;
    edge_count = 0;
    all_count = 0;
    for (x = 0; x <= xmax + 1; x++) 
    {
        if ((y2 - y2_new) >= ty) 
        {
            y2 -= ty;
            y -= 1;
            ty -= 2;
        }
        if (cx + x > 0 && cx + x < width && cy + y > 0 && cy + y < height)
        {
            all_count += 1;
            if (combined[cy+y][cx+x].get_red() == 1)
            {
                edge_count += 1;
            }
        }
        
        if (cx + x > 0 && cx + x < width && cy - y > 0 && cy - y < height)
        {
            all_count += 1;
            if (combined[cy-y][cx+x].get_red() == 1)
            {
                edge_count += 1; 
            }
        }

        if (cx - x > 0 && cx - x < width && cy + y > 0 && cy + y < height)
        {
            all_count += 1;
            if (combined[cy+y][cx-x].get_red() == 1)
            {
                edge_count += 1;
            }
        }

        if (cx - x > 0 && cx - x < width && cy - y > 0 && cy - y < height)
        {
            all_count += 1;
            if (combined[cy-y][cx-x].get_red() == 1)
            {
                edge_count += 1; 
            }
        }

        if (cx + y > 0 && cx + y < width && cy + x > 0 && cy + x < height)
        {
            all_count += 1;
            if (combined[cy+x][cx+y].get_red() == 1)
            {
                edge_count += 1;
            }
        }

        if (cx + y > 0 && cx + y < width && cy - x > 0 && cy - x < height)
        {
            all_count += 1;
            if (combined[cy-x][cx+y].get_red() == 1)
            {
                edge_count += 1;
            }
        }

        if (cx - y > 0 && cx - y < width && cy + x > 0 && cy + x < height)
        {
            all_count += 1;
            if (combined[cy+x][cx-y].get_red() == 1)
            {
                edge_count += 1;
            }
        }

        if (cx - y > 0 && cx - y < width && cy - x > 0 && cy - x < height)
        {
            all_count += 1;
            if (combined[cy-x][cx-y].get_red() == 1)
            {
                edge_count += 1;
            }
        }
        all_count += 1;
        y2_new -= (2 * x) - 3;
    }
    return edge_count/all_count;
}

/*
int find_radius(int cx, int cy, int height, int width)
{
    
}
*/

vector<vector<Pixel>> identify_circles(int rmin, int rmax, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row;
        for (int j = 0; j < width; j++)
        {
            Pixel copy_pixel;
            copy_pixel = input[i][j];
            int copy_red = copy_pixel.get_red();
            int copy_green = copy_pixel.get_green();
            int copy_blue = copy_pixel.get_blue();
            Pixel pixel;
            pixel.set_blue(copy_blue);
            pixel.set_green(copy_green);
            pixel.set_red(copy_red);
            row.push_back(pixel);
        }
        identified_circles.push_back(row);
    }
    int red_pixel_count, circles_drawn;
    red_pixel_count = 0;
    circles_drawn = 0;
    unordered_map<pair<int, int>, int, hash_pair> radii; 
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = centers[i][j];
            if (pixel.get_red() == 255 && pixel.get_blue() == 0 && pixel.get_green() == 0)
            {
                red_pixel_count += 1;
                for (int r = rmin; r < rmax; r++)
                {
                    // gets ratio of edges / total pixels
                    double ratio = count_circle(r, j, i);
                    if (ratio >= 0.20)
                    {
                        draw_circle(r, j, i, "identified_circles");
                    }
                }
            }
        }
    }
    cout << "Number of red pixels: " + to_string(red_pixel_count) + "\n";
    cout << "Drawn circles: " + to_string(circles_drawn) + "\n";
    return identified_circles;
}

void part3(int lower_t, int upper_t, string image, int circle_t)
{
    // reads in image ppm
    ifstream input_ppm;
    input_ppm.open(image);

    string fileType, hw, hex;

    getline(input_ppm, fileType);
    getline(input_ppm, hw);
    getline(input_ppm, hex);

    int ind;
    ind = hw.find(" ");
    width = stoi(hw.substr(0, ind));
    height = stoi(hw.substr(ind + 1));

    // gets every single number in ppm and puts into vector
    vector<int> all_nums;
    string line;
    while(getline(input_ppm, line))
    {
        string number = "";
        for (int i = 0; i < line.length(); i++)
        {
            if (line[i] == ' ')
            {
                all_nums.push_back(stoi(number));
                number = "";
            } 
            else
            {
                number.push_back(line[i]);
            }
        }
        if (number != "")
        {
            all_nums.push_back(stoi(number));
        }
    }

    int count;
    vector<Pixel> row;
    // organizes all numbers into vector of vectors of pixels
    count = 0;
    for (int i = 0; i < all_nums.size(); i+=3)
    {
        Pixel px;
        px.set_red(all_nums[i]);
        px.set_green(all_nums[i+1]);
        px.set_blue(all_nums[i+2]);
        row.push_back(px);
        count += 1;
        if (count == width)
        {
            count = 0;
            input.push_back(row);
            row = {};
        }
    }

    // gets grayscale, gx, gy, magnitude, and double threshold/hysteresis
    grayscale = getGrayscale(input, height, width);
    gx = getGx(grayscale, height, width);
    gy = getGy(grayscale, height, width);
    magnitude = getMagnitude(gx, gy, height, width);
    //applies double threshold + hysteresis
    edge = double_threshold(magnitude, height, width, lower_t, upper_t);
    h1 = hysteresis(height, width);
    h2 = replace(h1, height, width);

    ofstream grayscaleppm;
    grayscaleppm.open("imageg.ppm");
    grayscaleppm << "P3\n" + to_string(width) + " "  + to_string(height) + "\n255\n";
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = grayscale[i][j];
            grayscaleppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
        }
        grayscaleppm << "\n";
    }
    grayscaleppm.close();

    // converts to ppm
    ofstream h2ppm;
    h2ppm.open("image1.ppm");
    h2ppm << "P3\n" + to_string(width) + " "  + to_string(height) + "\n1\n";
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = h2[i][j];
            h2ppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
        }
        h2ppm << "\n";
    }
    h2ppm.close();

    // gets the rounded angles
    angles = angle_vector(gx, gy, height, width);

    // non max suppression algorithm
    suppression = non_max_suppression(angles, magnitude, height, width);

    // converst to ppm
    ofstream sppm;
    sppm.open("image2.ppm");
    sppm << "P3\n" + to_string(width) + " "  + to_string(height) + "\n1\n";
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = suppression[i][j];
            sppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
        }
        sppm << "\n";
    }
    sppm.close();

    combined = combined_algorithm(h2, suppression, height, width);
    ofstream cppm;
    cppm.open("imagef.ppm");
    cppm << "P3\n" + to_string(width) + " "  + to_string(height) + "\n1\n";
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel;
            pixel = combined[i][j];
            cppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
        }
        cppm << "\n";
    }
    cppm.close();
    cout << "Combined algorithm done. \n";
    unrounded_angles = unrounded_angle_vector(gx, gy, height, width);
    votes = voting(combined, unrounded_angles, height, width);
    cout << "Voting finished. \n";
    int max_num = vector_max(votes, height, width);

    centers = find_centers(circle_t, height, width);
    cout << "Centers found. \n";
    // lines
    ofstream vppm;
    vppm.open("imagev.ppm");
    vppm << "P3\n" + to_string(width) + " " + to_string(height) + "\n" + to_string(max_num) + "\n";
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                Pixel pixel;
                pixel = votes[i][j];
                vppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
            }
            vppm << "\n";
        }
        vppm.close();
    }

    ofstream ccppm;
    ccppm.open("imageCC.ppm");
    ccppm << "P3\n" + to_string(width) + " " + to_string(height) + "\n" + "255" + "\n";
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                Pixel pixel;
                pixel = centers[i][j];
                ccppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
            }
            ccppm << "\n";
        }
        ccppm.close();
    }
    
    identified_circles = identify_circles(80, 110, height, width);
    cout << "Circle vector created. \n";
    ofstream cirppm;
    cirppm.open("imagecir.ppm");
    cirppm << "P3\n" + to_string(width) + " " + to_string(height) + "\n" + "255" + "\n";
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                Pixel pixel;
                pixel = identified_circles[i][j];
                cirppm << to_string(pixel.get_red()) + " " + to_string(pixel.get_green()) + " " + to_string(pixel.get_blue()) + " ";
            }
            cirppm << "\n";
        }
        cirppm.close();
    }
    cout << "Circles ppm created. \n";
    //unordered_map<pair<int, int>, int> um;
    //um = identify_circles(50, 55, height, width);
}

int main(int argc,char* argv[])
{
    string name;
    int lower_t, upper_t, circle_t;
    if (argc == 1)
    {
        name = "image.ppm";
        lower_t = 90;
        upper_t = 200;
        circle_t = 60;
    }
    // reads command line inputs
    else
    {
        lower_t = stoi(argv[2]);
        upper_t = stoi(argv[4]);
        name = argv[6];
        circle_t = stoi(argv[8]);
    }
    
    
    part3(lower_t, upper_t, name, circle_t);
}
