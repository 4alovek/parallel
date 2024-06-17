#include <iostream>
#include <string>
#include <cmath>

double area(int area_start, int area_stop, int N)
{
    double s = 0;
    double height1{};
    double height2{};
    for (int i = area_start; i < area_stop; ++i)
    {
        height1 = sqrt(4.0 - pow((i*(2.0/N)), 2));
        height2 = sqrt(4.0 - pow(((i+1)*(2.0/N)), 2));
        s = s + (height1 + height2)/2* (2.0 / N);
    }
    return s;
}

int main()
{
    double s1 = area(0, 3333, 10000);
    double s2 = area(3333, 6666, 10000);
    double s3 = area(6666, 9999, 10000);
    std::cout << s1 << '\t' << s2 << '\t' << s3 << std::endl;
}