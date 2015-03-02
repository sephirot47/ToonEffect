#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>

const int WinWidth = 700, WinHeight = 700;
float edgeThreshold = 0.15f, colorStep = 40;
const sf::Color edgeColor = sf::Color::Black, clearColor = sf::Color::White;
std::string ImagePath = "face.png";

//Returns value 0.0 being the darkest, 1.0 being the brightest
inline float GetBrightness(const sf::Color &c) { return (0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b) / 255.0; }
inline bool GoodCoords(const sf::Image &img, int x, int y) { return x >= 0 and y >= 0 and x < img.getSize().x and y < img.getSize().y; }

bool IsEdge(const sf::Image &img, int x, int y)
{
    float bright = GetBrightness(img.getPixel(x,y));
    if(GoodCoords(img, x+1, y  ) and bright - GetBrightness(img.getPixel(x+1, y  )) > edgeThreshold)   return true;
    if(GoodCoords(img, x+1, y+1) and bright - GetBrightness(img.getPixel(x+1, y+1)) > edgeThreshold)   return true;
    if(GoodCoords(img, x-1, y-1) and bright - GetBrightness(img.getPixel(x-1, y-1)) > edgeThreshold)   return true;
    if(GoodCoords(img, x-1, y  ) and bright - GetBrightness(img.getPixel(x-1, y  )) > edgeThreshold)   return true;
    if(GoodCoords(img, x-1, y+1) and bright - GetBrightness(img.getPixel(x-1, y+1)) > edgeThreshold)   return true;
    if(GoodCoords(img, x  , y-1) and bright - GetBrightness(img.getPixel(x  , y-1)) > edgeThreshold) return true;
    if(GoodCoords(img, x  , y+1) and bright - GetBrightness(img.getPixel(x  , y+1)) > edgeThreshold) return true;
    if(GoodCoords(img, x+1, y-1) and bright - GetBrightness(img.getPixel(x+1, y-1)) > edgeThreshold)   return true;
    if(GoodCoords(img, x+1, y  ) and bright - GetBrightness(img.getPixel(x+1, y  )) > edgeThreshold)   return true;
    if(GoodCoords(img, x+1, y+1) and bright - GetBrightness(img.getPixel(x+1, y+1)) > edgeThreshold)   return true;
    return false;
}

sf::Image* GetEdgesImage(const sf::Image &img)
{
    sf::Image *result = new sf::Image();
    result->create(img.getSize().x, img.getSize().y);
    for(int i = 0; i < img.getSize().x; ++i)
    {
        for(int j = 0; j < img.getSize().y; ++j)
        {
            if(IsEdge(img, i, j)) result->setPixel(i, j, edgeColor);
            else result->setPixel(i, j, sf::Color(0, 0, 0, 0));
        }
    }
    return result;
}

unsigned int Discret(int x, int step, int min, int max)
{
    int below, above;
    below = (x / step) * step;
    above = below + step;
    if(x - below > above - x) x = above; else x = below;
    if(x > max) x = max; else if(x < min) x = min;
    return x;
}

sf::Color DiscretColor(sf::Color c, int step)
{
    return sf::Color(Discret(c.r, step, 0, 255), Discret(c.g, step, 0, 255), Discret(c.b, step, 0, 255), c.a);
}
sf::Color GetMeanColorAround(sf::Image &img, int i, int j, int radius)
{
    float div = 0, colr = 0, colg = 0, colb = 0, cola = 0;
    for(int k = -radius/2; k < radius*2; ++k)
    {
        for(int m = -radius/2; m < radius*2; ++m)
        {
            if(k+i >= img.getSize().x || k+i < 0 || m+j >= img.getSize().y || m+j < 0) continue;
            ++div;
            sf::Color imgCol = img.getPixel(k+i, m+j);
            colr += imgCol.r;
            colg += imgCol.g;
            colb += imgCol.b;
            cola += imgCol.a;
        }
    }
    sf::Color col((int)(colr/div), (int)(colg/div), (int)(colb/div), (int)(cola/div));
    return col;
}

void AntialiasImage(sf::Image &img)
{
    sf::Image auxImg;
    sf::IntRect r(0, 0, img.getSize().x, img.getSize().y);
    auxImg.copy(img, 0, 0, r);

    for(int i = 0; i < img.getSize().x; ++i)
    {
        for(int j = 0; j < img.getSize().y; ++j)
        {
            sf::Color imgColor = GetMeanColorAround(img, i, j, 1);
            img.setPixel(i, j, imgColor);
        }
    }
}

sf::Color BlendColor(sf::Color c1, sf::Color c2)
{
    sf::Color res(0, 0, 0, 0);
    res.r = c1.r * (255.0-c2.a)/255 + c2.r * float(c2.a)/255;
    res.g = c1.g * (255.0-c2.a)/255 + c2.g * float(c2.a)/255;
    res.b = c1.b * (255.0-c2.a)/255 + c2.b * float(c2.a)/255;
    res.a = c1.a * (255.0-c2.a)/255 + c2.a * float(c2.a)/255;
    return res;
}

sf::Image* GetBlendedImage(const sf::Image &img, const sf::Image &edgeImage)
{
    sf::Image *result = new sf::Image();
    result->create(img.getSize().x, img.getSize().y);
    for(int i = 0; i < img.getSize().x; ++i)
    {
        for(int j = 0; j < img.getSize().y; ++j)
        {
            if(edgeImage.getPixel(i,j).a > 0)
            {
                result->setPixel(i, j, edgeColor);
                sf::Color imgColor = GetMeanColorAround(*result, i, j, 1);
                result->setPixel(i, j, imgColor);
            }
            else
            {
                sf::Color imgColor = img.getPixel(i,j);
                imgColor = DiscretColor(imgColor, colorStep);
                result->setPixel(i, j, imgColor);
            }
        }
    }
    return result;
}


int main()
{
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), "SFML works!");

    sf::Image img, *edgesImg, *blendedImg;
    img.loadFromFile(ImagePath);
    sf::Texture tex, edgesTex, blendedTex;
    tex.loadFromImage(img);
    sf::Sprite sp(tex);
    sp.setTexture(tex);

    while (window.isOpen())
    {
        edgesImg = GetEdgesImage(img);
        edgesTex.loadFromImage(*edgesImg);
        sf::Sprite edgeSprite(edgesTex);
        edgeSprite.setPosition(img.getSize().x, 0.0f);

        blendedImg = GetBlendedImage(img, *edgesImg);
        blendedTex.loadFromImage(*blendedImg);
        sf::Sprite blendedSprite(blendedTex);
        blendedSprite.setPosition(img.getSize().x, 0);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(clearColor);
        window.draw(sp);
        //window.draw(edgeSprite);
        window.draw(blendedSprite);
        window.display();
    }
    return 0;
}
